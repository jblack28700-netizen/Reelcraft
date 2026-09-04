#pragma once

#include <QList>
#include <QString>

// A single oriented marker inside a deterministic 360-degree test scene.
//
// Angles use the same conventions as ViewportState:
//   - yaw is normalized to the range [-180, 180) degrees
//   - pitch is clamped to the range [-90, 90] degrees
//
// Markers are pure data and carry no camera, media, or rendering behavior.
struct ViewerSceneMarker
{
    double yaw = 0.0;   // degrees
    double pitch = 0.0; // degrees
    QString label;
};

// ViewerScene is a deterministic, synthetic 360-degree test scene.
//
// It deliberately contains no real media, no camera-specific logic, and no
// rendering code. It exists so the future 360 viewer presentation can be
// developed and verified against a known, reproducible set of oriented
// markers before any real media or camera integration exists.
class ViewerScene
{
public:
    ViewerScene() = default;

    // Returns the canonical deterministic test scene. Calling this repeatedly
    // produces identical scenes, which is what automated verification relies on.
    static ViewerScene createDeterministicTestScene();

    // True when every marker is within the canonical angle ranges and labels
    // are unique and non-empty.
    bool isValid() const;

    int markerCount() const { return m_markers.size(); }
    const QList<ViewerSceneMarker> &markers() const { return m_markers; }

    // Index of the first marker with the given label, or -1 when absent.
    int markerIndex(const QString &label) const;

private:
    // Adds a marker, normalizing/clamping its angles into the canonical ranges.
    void addMarker(double yaw, double pitch, const QString &label);

    QList<ViewerSceneMarker> m_markers;
};
