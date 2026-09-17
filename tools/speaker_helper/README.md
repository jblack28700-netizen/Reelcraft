# Reelcraft External Speaker/Audio Helper (Silero VAD)

This directory holds an **optional** audio/speaker helper for Reelcraft's
replaceable `SpeakerEvidenceProvider` / `ProcessSpeakerProvider` boundary. The
C++ core links no audio/ML runtime; the helper speaks the documented file/JSON
protocol and can be replaced freely.

## Selected provider

| Property | Value |
|---|---|
| Component | Silero VAD (`silero_vad.onnx`) |
| Source | https://github.com/snakers4/silero-vad |
| Code license | MIT |
| Model/weights license | MIT (the ONNX model is in the MIT repository) |
| Commercial use | Permitted |
| Runtime | Python 3 + ONNX Runtime (MIT); FFmpeg for audio decode |
| Input | 16 kHz mono PCM, 512-sample chunks with 64-sample context |
| Output | Speech intervals (activity), provider-local speaker id `spk1` |

**Why:** Silero VAD is a small, local, CPU-friendly, permissively licensed
speech-activity detector with no cloud/API dependence. It is a defensible first
provider behind the replaceable seam.

**What it is not:** VAD is not speaker diarization or audio-visual active-speaker
detection. It reports *when* speech occurs, not *who* is speaking. Reelcraft
associates the speech with a visible track using an explicit creator binding,
an optional provider-supplied direction of arrival, or the single-visible-person
rule; otherwise the result stays unassociated/ambiguous.

**Rejected/deferred alternatives:**
- pyannote segmentation/diarization: strong, but the models are gated behind
  access conditions and would add a PyTorch stack; deferred.
- SpeechBrain ECAPA / OSNet-style speaker embeddings: heavier PyTorch runtime and
  research-dataset (VoxCeleb) weight provenance; deferred pending review.
- Cloud speaker APIs: rejected (no cloud/API dependence requirement).
- Audio-visual active-speaker models (TalkNet/LoCoNet/AV-HuBERT): research-grade
  and/or unclear weight licensing; deferred.
- Objective 7 (2026-09-17) investigated automatic attribution directly: the real
  footage audio is mono (no direction of arrival), a face-detection +
  mouth-motion/audio-envelope correlation probe was not a reliable
  discriminator (0.250 speaker vs 0.192 listener), and no permissively
  licensed, clearly commercial audio-visual active-speaker model was found. The
  provider seam was therefore completed (see below) without shipping a
  model-backed provider. Decision 024.

## Install

```bash
# Runtime (Debian/Ubuntu example; OpenCV not required for this helper)
apt-get install -y --no-install-recommends python3-onnxruntime python3-numpy

# Model (download once; not committed)
mkdir -p ~/.cache/reelcraft/models
curl -L -o ~/.cache/reelcraft/models/silero_vad.onnx \
  https://raw.githubusercontent.com/snakers4/silero-vad/master/src/silero_vad/data/silero_vad.onnx
```

Set `REELCRAFT_SILERO_MODEL` to the model path (or pass `--model`) and
`REELCRAFT_FFMPEG` to the FFmpeg executable.

## Protocol

```
silero_vad_helper.py <request.json> <response.json> [--model PATH]
```

`request.json`:

```json
{ "media": "/path/clip.mp4", "startMs": 114000, "endMs": 126000 }
```

`response.json`:

```json
{ "available": true, "provider": "silero-vad", "startMs": 0, "endMs": 12000,
  "intervals": [ { "startMs": 0, "endMs": 2100, "speakerId": "spk1",
                   "confidence": 0.92, "overlap": false } ], "error": "" }
```

## Provider attribution hint (future audio-visual / diarization providers)

A provider that can name the *person* (diarization + face mapping, or an
audio-visual active-speaker model) may add an optional `targetIdHint` to each
interval — the id of an existing target track it attributes:

```json
{ "startMs": 0, "endMs": 2100, "speakerId": "spk1", "confidence": 0.92,
  "overlap": false, "targetIdHint": "t2" }
```

`SpeakerTargetAssociator` honours the hint only when that target is visible and
only after an explicit creator binding: explicit creator/structured binding >
visible provider hint (`provider-hint`) > spatial direction-of-arrival >
single-visible-person > ambiguous/unassociated. A hint for a non-visible target
falls through, and a hint never invents a target or overrides the creator. The
hint is carried through `SpeakerTimeline` onto the coalesced segment.

## Limitations

- Speech activity only; no speaker identity, diarization, or audio-visual
  association.
- CPU inference; one process per range (intentionally not optimized).
- Thresholds (`THRESHOLD=0.5`, minimum interval 100 ms, merge gap 200 ms) are
  documented constants; temporal hysteresis is owned by the C++ `SpeakerTimeline`.
- If the media has no decodable audio, the helper returns `available: false`;
  Reelcraft then keeps its geometry/appearance behavior unchanged.
