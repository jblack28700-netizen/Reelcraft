# Reelcraft — Target Resolution Technology Evaluation

## 1. Purpose

This document records the detection/tracking technology evaluation behind
360 Reframing Objective 2 (Target/Subject Resolution). It is the required
record of:

- candidate project/model name and source;
- license;
- whether commercial use is permitted;
- whether model weights carry separate licensing;
- relevant restrictions;
- why a candidate was selected or rejected.

It also records the chosen **dependency stance** so no dependency is introduced
silently or with unclear commercial terms.

## 2. Problem Statement

A normal 2D detector applied directly to a full equirectangular frame performs
poorly near the poles and the 0°/360° seam because equirectangular projection
distorts scale and shape strongly at high latitudes and duplicates the seam
boundary. Reelcraft needs target directions (yaw/pitch) usable by the existing
deterministic reframing engine (`ReframePlan` / `CameraPath` / `ReframeRenderer`).

## 3. Chosen Detection Strategy (evidence-based)

**Detect in overlapping perspective (tangent) views, then reproject detections
back to the sphere.**

The sphere is covered by a deterministic set of pinhole cameras
(`EquirectViewPlan`), each rendered with the *existing, already-tested*
`EquirectView` projection. A detector runs on each undistorted perspective
view. Each detection box is mapped to a spherical centre and angular extent
(`EquirectProjection::detectionToDirection`) using the exact inverse of the
established camera model. Observations close together (e.g. the same target in
overlapping views) are merged with spherical non-maximum suppression.

This is a standard, well-supported approach in the 360° detection literature
and industry practice (tangent-view / cubemap-facet detection with
reprojection; e.g. "Six-to-one: Cubemap-guided Feature Calibration for
Panorama Object Detection", ICTAI 2022, and spherical-criteria work on 360°
object detection). It avoids training a custom spherical model while producing
correct spherical coordinates, and it keeps the detector itself replaceable.

Rejected for the first slice:

- **Direct equirectangular detection:** cheap but distorted; unacceptable
  accuracy near poles/seam.
- **Custom spherical/equirect convolution model:** research-grade, large
  training burden, no mature permissively licensed off-the-shelf model.
- **Cubemap-only pipeline:** viable, but tangent views reuse the existing,
  tested `EquirectView` primitive with no new projection code.

## 4. Candidate Detector/Model Evaluation

| Candidate | Source | License | Commercial use | Weights licensing | Notes |
|---|---|---|---|---|---|
| **YOLOX** | github.com/Megvii-BaseDetection/YOLOX | Apache-2.0 | Yes | Apache-2.0 (Megvii) | Permissive; strong small/medium models; ONNX export. **Acceptable.** |
| **RT-DETR** | github.com/lyuwenyu/RT-DETR | Apache-2.0 | Yes | Apache-2.0 | Real-time DETR; permissive. **Acceptable.** |
| **OpenCV / OpenCV Zoo** | github.com/opencv/opencv_zoo | Apache-2.0 | Yes | Per-model; check each model card (many Apache-2.0) | dnn module + small ONNX models; YuNet face detection. **Acceptable**, verify per-model. |
| **MediaPipe (Tasks)** | github.com/google-ai-edge/mediapipe | Apache-2.0 | Yes | Apache-2.0 (Google) | Face/person/pose/object TFLite tasks. **Acceptable.** |
| **ONNX Runtime** | github.com/microsoft/onnxruntime | MIT | Yes | n/a (runtime) | Runtime for the above; permissive. **Acceptable.** |
| **Ultralytics YOLO (v5/v8/v11/…)** | github.com/ultralytics/ultralytics | **AGPL-3.0** | Only with a paid Ultralytics Enterprise License for proprietary/closed-source/SaaS use | Model weights follow the project license | **Rejected as a default dependency**: AGPL would force open-sourcing Reelcraft or require an enterprise license. May be used only if the creator explicitly accepts that obligation. |
| **YOLO-NAS (Deci)** | Deci | Restrictive (non-commercial terms on weights/model) | Not clearly permitted | Separate restrictive terms | **Rejected**: commercial-use status is not acceptable/clear. |

Sources were verified from the projects' own LICENSE files and the Ultralytics
licensing page during this objective (2026-09-17).

## 5. Chosen Dependency Stance

For this objective Reelcraft **introduces no linked computer-vision dependency**.
The detection boundary (`TargetDetector`) is a pure interface, and the
production-capable adapter (`ProcessTargetDetector`) talks to an **external
helper process** over a small file/JSON protocol. This means:

- the core build remains dependency-free and license-clean;
- a model/runtime (YOLOX, RT-DETR, OpenCV Zoo, MediaPipe, ONNX Runtime) can be
  added and replaced without changing Reelcraft;
- the helper's license and weights can be audited independently before it ships;
- the interface does not assume a specific object taxonomy.

The permissive candidates above are the recommended helper stack. A helper
using AGPL Ultralytics models must not be bundled without the Enterprise
License; that is a product/legal decision, not an implementation detail.

## 6. Helper Protocol

    <executable> [arguments...] <request.json> <response.json>

request.json:

    {
      "image": "<absolute path to a PNG perspective view>",
      "width": 320,
      "height": 240,
      "query": { "label": "person", "targetId": "", "minConfidence": 0.3 }
    }

response.json:

    {
      "detections": [
        { "x": 10, "y": 20, "width": 30, "height": 40,
          "label": "person", "confidence": 0.91, "id": "optional-detector-id" }
      ]
    }

Coordinates are in view pixels. Malformed detections are skipped; a missing
`detections` array, non-zero exit, or timeout is a deterministic error.

## 7. Tracking

The first tracker is `SphericalTargetTracker`: deterministic spherical
non-maximum suppression within a frame plus greedy nearest-neighbour
association on great-circle distance with a gate and miss counting. It is
identity-persistent but has no appearance/re-identification model. A stronger
tracker (motion prediction, embeddings, face identity) can replace it behind
the same class.

## 8. Not Solved Here

- "Me" identity (which person is the creator) — requires re-identification or a
  creator-selected seed; not solved.
- Speaker localization / active-speaker detection.
- Semantic understanding and open-vocabulary classes beyond the detector's.
- Bundling an actual model/helper and its weights.
- Production tracking quality.

## 9. Status

- Detection strategy: **decided** (tangent views + spherical reprojection).
- Dependency stance: **decided** (no linked CV dependency; subprocess seam).
- Real model integration: **implemented and verified** (2026-09-17) with an
  optional external helper using OpenCV Zoo YOLOX + OpenCV DNN on CPU; see
  Section 10.

## 10. Real Integration Result (2026-09-17, Objective 3)

**Selected:** OpenCV Zoo `object_detection_yolox_2022nov.onnx` (Apache-2.0,
code and weights), COCO 80 classes. **Runtime:** Python 3 + OpenCV DNN 4.10
(`python3-opencv`), CPU. The helper (`tools/detector_helper/yolox_detector.py`)
implements the Section 6 protocol; Reelcraft links no CV library.

**Install:** `apt-get install -y --no-install-recommends python3-opencv
python3-numpy`, then download the ONNX weights (~35.8 MB) to
`~/.cache/reelcraft/models/`. **GPU:** OpenCV DNN can use CUDA/TIM-VX/CANN
backends; this run used CPU because no RunPod credentials were available.

**Verified on real footage:** the project's real `360_TEST_4K.mp4`
(3840x1920 VP9, 2:1 equirect). A 1920x960 x264 proxy of the 114-126 s segment
was used for speed; the original was never modified.

- Full equirect sanity check (t=120 s): 4 `person` detections, top-2 at
  confidence 0.89 (two presenters).
- Tangent-view pipeline (`EquirectView` front view, fov 110): detections
  mapped to spherical directions (presenter A: yaw -27.9, pitch -28.6;
  presenter B: yaw +26.9, pitch -27.8). A diagnostic reproduction of the same
  projection produced identical coordinates, confirming the C++ mapping.
- Tracker: 9 raw tracks across 5 sampled frames (5.5-7.5 s proxy time); the
  strongest `person` track held a stable id `t1` across all five frames
  (mean confidence 0.923, angular extent ~14x38 degrees).
- Reframe: the track produced a validated `ReframePlan` (5 keyframes) and
  `ReframeRenderer` produced a 640x360 H.264 clip whose frames visibly center
  the detected presenter.

**Limitations observed:** the two presenters were close in yaw (about 55
degrees apart) and stayed static, so the simple nearest-neighbour tracker did
not have to disambiguate motion; small background people were detected at
lower confidence and formed short extra tracks; one model load per helper
invocation (process-per-view) is intentionally unoptimized.
