# Reelcraft External Appearance/ReID Helper

This directory holds an **optional** appearance-inference helper for Reelcraft's
replaceable `AppearanceProvider` / `ProcessAppearanceProvider` boundary. The C++
core links no Python, OpenCV, ONNX Runtime, or model; the helper speaks the
documented file/JSON protocol and can be replaced freely.

## Selected model

| Property | Value |
|---|---|
| Model | OpenVINO Open Model Zoo `person-reidentification-retail-0277` |
| ONNX file | `person-reidentification-retail-0265.onnx` (~9.6 MB) |
| Source | https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/person-reidentification-retail-0277/ |
| OMZ model page | https://github.com/openvinotoolkit/open_model_zoo/tree/master/models/intel/person-reidentification-retail-0277 |
| Code license | Apache-2.0 (Open Model Zoo) |
| Model/weights license | Apache-2.0 (the model `model.yml` references the OMZ LICENSE) |
| Commercial use | Permitted |
| Training data | Internal dataset (not Market-1501 / MSMT17 / DukeMTMC) |
| Embedding | 256-d |
| Input | 256x128 (HxW), NCHW, raw BGR 0-255 (the model normalizes internally) |
| Runtime | Python 3 + ONNX Runtime (MIT) |

**Why this model:** it is permissively licensed for both code and weights and is
trained on an internal dataset, avoiding the research-dataset weight-license
problems below.

**Rejected alternatives (weights licensing unclear / non-commercial):**
- OpenCV Zoo `person_reid_youtureid` — directory LICENSE is Apache-2.0, but the
  weights come from `ReID-Team/ReID_extra_testdata`, which has **no license**
  and is trained on Market1501/DukeMTMC/MSMT17 (research datasets).
- OSNet / torchreid weights — code is MIT but pretrained weights are trained on
  research datasets and are frequently unlicensed.
- Ultralytics YOLO (any task) — AGPL-3.0 / enterprise license (see
  `docs/TARGET_RESOLUTION_TECHNOLOGY.md`).

## Install

```bash
# Runtime (Debian/Ubuntu example)
apt-get install -y --no-install-recommends python3-onnxruntime python3-opencv

# Model weights (download once; not committed)
mkdir -p ~/.cache/reelcraft/models
curl -L -o ~/.cache/reelcraft/models/reid_0277.onnx \
  https://storage.openvinotoolkit.org/repositories/open_model_zoo/2023.0/models_bin/1/person-reidentification-retail-0277/person-reidentification-retail-0265.onnx
```

Set `REELCRAFT_REID_MODEL` to the model path, or pass `--model`.

## Protocol

```
reid_onnx_helper.py <request.json> <response.json> [--model PATH]
```

`request.json`:

```json
{ "image": "/abs/path/to/crop.png", "width": 128, "height": 256,
  "targetId": "t1", "timeMs": 5500 }
```

`response.json`:

```json
{ "embedding": [ ... 256 floats ... ], "dimension": 256,
  "quality": 1.0, "provider": "reid-retail-0277" }
```

Reelcraft L2-normalizes the embedding and compares with cosine similarity.

## GPU

The helper uses the ONNX Runtime CPU provider. A CUDA build can be selected on
RunPod later by changing the provider list; Reelcraft is runtime-independent.

## Limitations

- Person re-identification only (not face recognition); it is appearance
  similarity, not biometric certainty.
- Thresholds are dataset/domain dependent; Reelcraft ships configurable
  defaults (accept 0.75, reject 0.55) that were validated on the project's real
  360 footage.
- One model load per helper invocation (the protocol is process-per-crop);
  intentionally not optimized yet.
