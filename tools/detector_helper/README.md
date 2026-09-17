# Reelcraft External Detector Helper (YOLOX)

This directory holds an **optional** detector helper for Reelcraft's replaceable
`TargetDetector` / `ProcessTargetDetector` boundary. Reelcraft's C++ build does
not link Python, OpenCV, ONNX, or any model: the helper speaks the documented
file/JSON protocol and can be replaced freely.

## Selected model

| Property | Value |
|---|---|
| Model | OpenCV Zoo `object_detection_yolox` (2022nov) |
| File | `object_detection_yolox_2022nov.onnx` (~35.8 MB) |
| Source | https://github.com/opencv/opencv_zoo/tree/main/models/object_detection_yolox |
| Code license | Apache-2.0 |
| Model/weights license | Apache-2.0 (the model directory's `LICENSE`) |
| Commercial use | Permitted |
| Classes | COCO 2017 (80 classes); `person` is class 0 |
| Runtime | Python 3 + OpenCV DNN 4.10 (CPU) |

The inference/letterbox/postprocess code in `yolox_detector.py` is adapted from
the OpenCV Zoo demo (Apache-2.0). Ultralytics YOLO is intentionally not used
(AGPL-3.0 / enterprise licensing); see `docs/TARGET_RESOLUTION_TECHNOLOGY.md`.

## Install

```bash
# Runtime (Debian/Ubuntu example)
apt-get install -y --no-install-recommends python3-opencv python3-numpy

# Model weights (download once; not committed)
mkdir -p ~/.cache/reelcraft/models
curl -L -o ~/.cache/reelcraft/models/yolox_2022nov.onnx \
  https://github.com/opencv/opencv_zoo/raw/main/models/object_detection_yolox/object_detection_yolox_2022nov.onnx
```

Set `REELCRAFT_YOLOX_MODEL` to the model path, or pass `--model`.

## Protocol

```
yolox_detector.py <request.json> <response.json>
```

`request.json`:

```json
{
  "image": "/abs/path/to/perspective_view.png",
  "width": 512,
  "height": 512,
  "query": { "label": "person", "targetId": "", "minConfidence": 0.35 }
}
```

`response.json`:

```json
{
  "detections": [
    { "x": 200, "y": 120, "width": 80, "height": 240,
      "label": "person", "confidence": 0.89, "id": "" }
  ]
}
```

Coordinates are in the perspective view's pixels; Reelcraft maps them to
spherical yaw/pitch with `EquirectProjection` (the exact inverse of
`EquirectView`).

## GPU

The helper currently uses the OpenCV DNN OpenCV-backend CPU target. OpenCV DNN
also supports CUDA, TIM-VX, and CANN backends; a GPU build (for example on
RunPod) can be selected by editing the backend/target in `yolox_detector.py`.
Reelcraft itself is runtime-independent.

## Limitations

- COCO 80 classes only; no open-vocabulary or person re-identification.
- Small, distant, occluded, or strongly distorted people may be missed;
  detection runs on undistorted tangent views to reduce distortion.
- One model load per invocation (the protocol is process-per-view). This is
  intentionally not optimized yet.
- No face recognition, speaker localization, or identity ("me") resolution.
