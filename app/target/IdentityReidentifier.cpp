#include "target/IdentityReidentifier.h"

#include "target/EquirectProjection.h"

#include <algorithm>
#include <climits>

namespace {

int trackNumber(const QString &id)
{
    if (id.size() >= 2 && id.at(0) == QLatin1Char('t')) {
        bool ok = false;
        const int number = id.mid(1).toInt(&ok);
        if (ok) {
            return number;
        }
    }
    return INT_MAX;
}

const TargetTrack *findTrack(const QList<TargetTrack> &tracks, const QString &id)
{
    for (const TargetTrack &track : tracks) {
        if (track.id() == id) {
            return &track;
        }
    }
    return nullptr;
}

bool labelsCompatible(const QString &a, const QString &b)
{
    return a.isEmpty() || b.isEmpty() || a == b;
}

} // namespace

IdentityReidentifier::IdentityReidentifier(Config config)
    : m_config(std::move(config))
{
}

IdentityReidentifier::Config IdentityReidentifier::config() const
{
    return m_config;
}

void IdentityReidentifier::setConfig(const Config &config)
{
    m_config = config;
}

AppearanceVerdict IdentityReidentifier::classify(double similarity) const
{
    if (similarity >= m_config.acceptThreshold) {
        return AppearanceVerdict::Agree;
    }
    if (similarity < m_config.rejectThreshold) {
        return AppearanceVerdict::Disagree;
    }
    return AppearanceVerdict::Weak;
}

bool IdentityReidentifier::encodeObservation(
    const TargetObservation &observation, ReframeFrameProvider *frames,
    AppearanceProvider *provider, AppearanceEmbedding *out, QString *error) const
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!frames || !provider) {
        return fail(QStringLiteral("Appearance provider or frame source is missing."));
    }
    if (!observation.isValid()) {
        return fail(QStringLiteral("Appearance observation is invalid."));
    }

    QImage equirect;
    QString frameError;
    if (!frames->frameAt(observation.timeMs, &equirect, &frameError)) {
        return fail(QStringLiteral("Appearance frame unavailable: %1").arg(frameError));
    }
    QImage crop;
    QString cropError;
    if (!TargetCropExtractor::crop(equirect, observation.yawDeg, observation.pitchDeg,
                                   observation.yawRadiusDeg,
                                   observation.pitchRadiusDeg, m_config.crop,
                                   &crop, &cropError)) {
        return fail(QStringLiteral("Appearance crop failed: %1").arg(cropError));
    }
    return provider->encode(crop, observation.targetId, observation.timeMs, out, error);
}

bool IdentityReidentifier::buildProfile(const TargetTrack &track,
                                        ReframeFrameProvider *frames,
                                        AppearanceProvider *provider, qint64 timeMs,
                                        AppearanceProfile *out,
                                        QString *error) const
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Appearance profile output is null."));
    }
    if (track.isEmpty()) {
        return fail(QStringLiteral("Cannot build an appearance profile from an empty track."));
    }

    QList<TargetObservation> samples;
    const int count = track.size();
    const int wanted = qMax(1, m_config.maxReferenceSamples);
    if (count <= wanted) {
        samples = track.observations();
    } else {
        for (int i = 0; i < wanted; ++i) {
            const int index = static_cast<int>(
                std::llround(static_cast<double>(i)
                             * static_cast<double>(count - 1)
                             / static_cast<double>(wanted - 1)));
            samples.append(track.observations().at(qBound(0, index, count - 1)));
        }
    }

    QList<AppearanceEmbedding> embeddings;
    for (const TargetObservation &observation : samples) {
        AppearanceEmbedding embedding;
        QString encodeError;
        if (encodeObservation(observation, frames, provider, &embedding,
                              &encodeError)) {
            embeddings.append(embedding);
        }
    }
    if (embeddings.isEmpty()) {
        return fail(QStringLiteral("No appearance crop could be encoded."));
    }

    AppearanceEmbedding reference;
    if (!AppearanceMath::aggregate(embeddings, &reference, error)) {
        return false;
    }
    AppearanceProfile profile;
    profile.reference = reference;
    profile.sampleCount = embeddings.size();
    profile.updatedAtMs = timeMs;
    profile.provider = provider->name();
    *out = profile;
    return true;
}

IdentityReidentifier::Result IdentityReidentifier::reidentify(
    TargetIdentityRegistry *registry, const QList<TargetTrack> &tracks,
    ReframeFrameProvider *frames, AppearanceProvider *provider, qint64 timeMs)
{
    Result result;
    if (!registry) {
        result.notes.append(QStringLiteral("No identity registry."));
        return result;
    }

    const QStringList changed = registry->update(tracks, timeMs);
    result.changedIdentities = changed;

    if (!provider || !frames) {
        result.notes.append(QStringLiteral(
            "Appearance provider unavailable; geometry-only identity."));
        return result;
    }

    const QStringList identities = registry->identities();
    for (const QString &identity : identities) {
        const IdentityBinding *binding = registry->binding(identity);
        if (!binding) {
            continue;
        }
        const TargetTrack *boundTrack = findTrack(tracks, binding->targetId);

        // Maintain the appearance profile while the identity is actively bound.
        if (binding->resolved && boundTrack && boundTrack->active()
            && !registry->hasAppearanceProfile(identity)) {
            AppearanceProfile profile;
            QString profileError;
            profile.identity = identity;
            if (buildProfile(*boundTrack, frames, provider, timeMs, &profile,
                             &profileError)) {
                registry->setAppearanceProfile(identity, profile);
                result.notes.append(QStringLiteral(
                    "Appearance profile for '%1' built from %2 sample(s).")
                                       .arg(identity)
                                       .arg(profile.sampleCount));
            } else {
                result.notes.append(QStringLiteral(
                    "Appearance profile for '%1' unavailable: %2")
                                       .arg(identity, profileError));
            }
        }

        const bool activeBinding =
            binding->resolved && boundTrack && boundTrack->active();

        if (activeBinding) {
            // Precedence 1/2: explicit or active binding is authoritative. Only a
            // geometric continuation is verified (and may be vetoed) by appearance.
            if (binding->method != QStringLiteral("continuity-rebind")) {
                continue;
            }
            const AppearanceProfile *profile = registry->appearanceProfile(identity);
            if (!profile) {
                result.notes.append(QStringLiteral(
                    "Geometry continuation for '%1' accepted; no appearance "
                    "profile to verify.").arg(identity));
                continue;
            }
            TargetObservation representative;
            if (!boundTrack->representative(&representative)) {
                continue;
            }
            AppearanceEmbedding embedding;
            QString encodeError;
            AppearanceEvidence evidence;
            evidence.identity = identity;
            evidence.candidateTrackId = binding->targetId;
            evidence.acceptThreshold = m_config.acceptThreshold;
            evidence.rejectThreshold = m_config.rejectThreshold;
            evidence.provider = provider->name();
            evidence.timeMs = timeMs;
            if (!encodeObservation(representative, frames, provider, &embedding,
                                   &encodeError)) {
                evidence.verdict = AppearanceVerdict::Unavailable;
                evidence.detail = encodeError;
                result.evidence.append(evidence);
                result.notes.append(QStringLiteral(
                    "Appearance unavailable to verify '%1': %2")
                                       .arg(identity, encodeError));
                continue;
            }
            double similarity = 0.0;
            QString similarityError;
            if (!AppearanceMath::cosineSimilarity(profile->reference, embedding,
                                                  &similarity, &similarityError)) {
                evidence.verdict = AppearanceVerdict::Unavailable;
                evidence.detail = similarityError;
                result.evidence.append(evidence);
                continue;
            }
            const AppearanceVerdict verdict = classify(similarity);
            evidence.similarity = similarity;
            evidence.verdict = verdict;
            evidence.detail = QStringLiteral("geometry/appearance verification");
            result.evidence.append(evidence);
            if (verdict == AppearanceVerdict::Disagree) {
                registry->rejectTarget(
                    identity, binding->targetId,
                    QStringLiteral("appearance disagrees with geometric continuation"));
                registry->annotateAppearance(identity, similarity, verdict,
                                             QStringLiteral("geometry/appearance conflict"));
                if (!result.changedIdentities.contains(identity)) {
                    result.changedIdentities.append(identity);
                }
                result.notes.append(QStringLiteral(
                    "Conflict for '%1': geometry -> %2 but appearance disagrees "
                    "(similarity %3). Identity left unresolved.")
                                       .arg(identity, binding->targetId)
                                       .arg(similarity, 0, 'f', 3));
            } else {
                registry->annotateAppearance(identity, similarity, verdict,
                                             QStringLiteral("geometry confirmed"));
                result.notes.append(QStringLiteral(
                    "Geometry continuation for '%1' confirmed by appearance "
                    "(similarity %2).")
                                       .arg(identity)
                                       .arg(similarity, 0, 'f', 3));
            }
            continue;
        }

        // Precedence 4/5: unresolved — attempt appearance re-acquisition.
        const AppearanceProfile *profile = registry->appearanceProfile(identity);
        if (!profile) {
            result.notes.append(QStringLiteral(
                "Identity '%1' unresolved; no appearance profile available.")
                                   .arg(identity));
            continue;
        }

        QList<TargetTrack> candidates;
        for (const TargetTrack &track : tracks) {
            if (!track.active() || track.isEmpty()
                || track.id() == binding->targetId) {
                continue;
            }
            if (!labelsCompatible(m_config.label, track.label())) {
                continue;
            }
            bool claimed = false;
            for (const QString &otherIdentity : identities) {
                if (otherIdentity == identity) {
                    continue;
                }
                const IdentityBinding *other = registry->binding(otherIdentity);
                if (other && other->targetId == track.id()) {
                    claimed = true;
                    break;
                }
            }
            if (!claimed) {
                candidates.append(track);
            }
        }
        std::stable_sort(candidates.begin(), candidates.end(),
                         [](const TargetTrack &a, const TargetTrack &b) {
                             if (a.firstTimeMs() != b.firstTimeMs()) {
                                 return a.firstTimeMs() < b.firstTimeMs();
                             }
                             const int numberA = trackNumber(a.id());
                             const int numberB = trackNumber(b.id());
                             if (numberA != numberB) {
                                 return numberA < numberB;
                             }
                             return a.id() < b.id();
                         });

        double bestSimilarity = -1.0;
        QList<QString> strongCandidates;
        QList<double> strongSimilarities;
        for (const TargetTrack &candidate : candidates) {
            TargetObservation representative;
            if (!candidate.representative(&representative)) {
                continue;
            }
            AppearanceEvidence evidence;
            evidence.identity = identity;
            evidence.candidateTrackId = candidate.id();
            evidence.acceptThreshold = m_config.acceptThreshold;
            evidence.rejectThreshold = m_config.rejectThreshold;
            evidence.provider = provider->name();
            evidence.timeMs = timeMs;

            AppearanceEmbedding embedding;
            QString encodeError;
            if (!encodeObservation(representative, frames, provider, &embedding,
                                   &encodeError)) {
                evidence.verdict = AppearanceVerdict::Unavailable;
                evidence.detail = encodeError;
                result.evidence.append(evidence);
                continue;
            }
            double similarity = 0.0;
            QString similarityError;
            if (!AppearanceMath::cosineSimilarity(profile->reference, embedding,
                                                  &similarity, &similarityError)) {
                evidence.verdict = AppearanceVerdict::Unavailable;
                evidence.detail = similarityError;
                result.evidence.append(evidence);
                continue;
            }
            evidence.similarity = similarity;
            evidence.verdict = classify(similarity);
            evidence.detail = QStringLiteral("candidate re-acquisition");
            result.evidence.append(evidence);
            bestSimilarity = qMax(bestSimilarity, similarity);
            if (evidence.verdict == AppearanceVerdict::Agree) {
                strongCandidates.append(candidate.id());
                strongSimilarities.append(similarity);
            }
        }

        if (strongCandidates.size() == 1) {
            QString rebindError;
            if (registry->rebindWithAppearance(
                    identity, strongCandidates.first(), strongSimilarities.first(),
                    QStringLiteral("single strong appearance match"), &rebindError)) {
                if (!result.changedIdentities.contains(identity)) {
                    result.changedIdentities.append(identity);
                }
                result.notes.append(QStringLiteral(
                    "Identity '%1' re-acquired track %2 by appearance "
                    "(similarity %3).")
                                       .arg(identity, strongCandidates.first())
                                       .arg(strongSimilarities.first(), 0, 'f', 3));
            } else {
                result.notes.append(QStringLiteral(
                    "Appearance re-acquisition failed for '%1': %2")
                                       .arg(identity, rebindError));
            }
        } else if (strongCandidates.size() > 1) {
            registry->annotateAppearance(identity, bestSimilarity,
                                         AppearanceVerdict::Ambiguous,
                                         QStringLiteral("multiple strong candidates"));
            result.notes.append(QStringLiteral(
                "Appearance ambiguous for '%1': %2 strong candidates (%3).")
                                   .arg(identity)
                                   .arg(strongCandidates.size())
                                   .arg(strongCandidates.join(QStringLiteral(", "))));
        } else {
            registry->annotateAppearance(
                identity, bestSimilarity,
                bestSimilarity < 0.0 ? AppearanceVerdict::Unavailable
                                     : classify(bestSimilarity),
                QStringLiteral("no strong appearance match"));
            result.notes.append(QStringLiteral(
                "No strong appearance match for '%1' (best similarity %2); "
                "remains unresolved.")
                                   .arg(identity)
                                   .arg(bestSimilarity < 0.0
                                            ? QStringLiteral("n/a")
                                            : QString::number(bestSimilarity, 'f', 3)));
        }
    }

    return result;
}
