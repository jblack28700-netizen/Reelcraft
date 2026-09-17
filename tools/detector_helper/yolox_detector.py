#!/usr/bin/env python3
"""Reelcraft external detector helper (ProcessTargetDetector protocol).

This is an OPTIONAL helper behind Reelcraft's replaceable TargetDetector seam.
It runs a real, permissively licensed object detector (OpenCV Zoo YOLOX,
Apache-2.0) on a perspective/tangent view produced by the Reelcraft C++
projection system, and returns detections as JSON.

It deliberately lives outside the Reelcraft build: the C++ application does
not link Python, OpenCV, ONNX, or any model. It communicates only through the
documented file/JSON protocol:

    yolox_detector.py <request.json> <response.json>

request.json:
    { "image": "<path to PNG>", "width": N, "height": N,
      "query": { "label": "person", "targetId": "", "minConfidence": 0.3 } }

response.json:
    { "detections": [ { "x":.., "y":.., "width":.., "height":..,
                        "label":"person", "confidence":0.9, "id":"" } ] }

Model:
    OpenCV Zoo object_detection_yolox (2022nov), Apache-2.0.
    Set REELCRAFT_YOLOX_MODEL to the .onnx path, or pass --model.
    See tools/detector_helper/README.md for the download and licensing record.

The YOLOX inference/letterbox/postprocess code is adapted from the OpenCV Zoo
demo (models/object_detection_yolox), Apache-2.0.
"""
import argparse
import json
import os
import sys

import cv2 as cv
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from coco_classes import COCO_CLASSES  # noqa: E402

DEFAULT_MODEL = os.environ.get(
    "REELCRAFT_YOLOX_MODEL",
    os.path.expanduser("~/.cache/reelcraft/models/yolox_2022nov.onnx"),
)


class YoloX:
    """Minimal OpenCV DNN YOLOX wrapper (adapted from OpenCV Zoo, Apache-2.0)."""

    def __init__(self, model_path, conf_threshold=0.35, nms_threshold=0.5,
                 obj_threshold=0.5):
        self.num_classes = 80
        self.net = cv.dnn.readNet(model_path)
        self.input_size = (640, 640)
        self.strides = [8, 16, 32]
        self.conf_threshold = conf_threshold
        self.nms_threshold = nms_threshold
        self.obj_threshold = obj_threshold
        self.net.setPreferableBackend(cv.dnn.DNN_BACKEND_OPENCV)
        self.net.setPreferableTarget(cv.dnn.DNN_TARGET_CPU)
        self._generate_anchors()

    def _generate_anchors(self):
        grids = []
        expanded_strides = []
        hsizes = [self.input_size[0] // s for s in self.strides]
        wsizes = [self.input_size[1] // s for s in self.strides]
        for hsize, wsize, stride in zip(hsizes, wsizes, self.strides):
            xv, yv = np.meshgrid(np.arange(hsize), np.arange(wsize))
            grid = np.stack((xv, yv), 2).reshape(1, -1, 2)
            grids.append(grid)
            expanded_strides.append(np.full((*grid.shape[:2], 1), stride))
        self.grids = np.concatenate(grids, 1)
        self.expanded_strides = np.concatenate(expanded_strides, 1)

    def infer(self, rgb_image):
        blob = np.transpose(rgb_image.astype(np.float32), (2, 0, 1))[np.newaxis, :, :, :]
        self.net.setInput(blob)
        outs = self.net.forward(self.net.getUnconnectedOutLayersNames())
        return self._postprocess(outs[0])

    def _postprocess(self, outputs):
        dets = outputs[0]
        dets[:, :2] = (dets[:, :2] + self.grids) * self.expanded_strides
        dets[:, 2:4] = np.exp(dets[:, 2:4]) * self.expanded_strides

        boxes = dets[:, :4]
        boxes_xywh = np.ones_like(boxes)
        boxes_xywh[:, 0] = boxes[:, 0] - boxes[:, 2] / 2.0
        boxes_xywh[:, 1] = boxes[:, 1] - boxes[:, 3] / 2.0
        boxes_xywh[:, 2] = boxes[:, 2]
        boxes_xywh[:, 3] = boxes[:, 3]

        scores = dets[:, 4:5] * dets[:, 5:]
        max_scores = np.amax(scores, axis=1)
        max_scores_idx = np.argmax(scores, axis=1)
        keep = cv.dnn.NMSBoxesBatched(
            boxes_xywh.tolist(), max_scores.tolist(), max_scores_idx.tolist(),
            self.conf_threshold, self.nms_threshold)
        if len(keep) == 0:
            return np.array([])
        candidates = np.concatenate(
            [boxes_xywh, max_scores[:, None], max_scores_idx[:, None]], axis=1)
        return candidates[keep]


def letterbox(src_bgr, target_size=(640, 640)):
    rgb = cv.cvtColor(src_bgr, cv.COLOR_BGR2RGB)
    padded = np.ones((target_size[0], target_size[1], 3), dtype=np.float32) * 114.0
    ratio = min(target_size[0] / rgb.shape[0], target_size[1] / rgb.shape[1])
    resized = cv.resize(rgb, (int(rgb.shape[1] * ratio), int(rgb.shape[0] * ratio)),
                        interpolation=cv.INTER_LINEAR).astype(np.float32)
    padded[: int(rgb.shape[0] * ratio), : int(rgb.shape[1] * ratio)] = resized
    return padded, ratio


def run(request_path, response_path, model_path, conf, nms, obj):
    with open(request_path, "r", encoding="utf-8") as fh:
        request = json.load(fh)

    image_path = request.get("image", "")
    if not image_path or not os.path.isfile(image_path):
        raise RuntimeError("request image does not exist: %r" % image_path)

    query = request.get("query", {}) or {}
    wanted_label = (query.get("label") or "").strip().lower()
    min_conf = float(query.get("minConfidence") or 0.0)

    image = cv.imread(image_path, cv.IMREAD_COLOR)
    if image is None:
        raise RuntimeError("could not decode image: %r" % image_path)

    detector = get_detector(model_path, conf, nms, obj)
    padded, scale = letterbox(image)
    predictions = detector.infer(padded)

    detections = []
    for det in predictions:
        x, y, w, h = det[:4]
        score = float(det[-2])
        cls_id = int(det[-1])
        if score < min_conf or score < conf:
            continue
        label = COCO_CLASSES[cls_id] if 0 <= cls_id < len(COCO_CLASSES) else str(cls_id)
        if wanted_label and label.lower() != wanted_label:
            continue
        detections.append({
            "x": float(x / scale),
            "y": float(y / scale),
            "width": float(w / scale),
            "height": float(h / scale),
            "label": label,
            "confidence": score,
            "id": "",
        })

    with open(response_path, "w", encoding="utf-8") as fh:
        json.dump({"detections": detections}, fh)


_CACHE = {}


def get_detector(model_path, conf, nms, obj):
    key = (model_path, conf, nms, obj)
    if key not in _CACHE:
        if not os.path.isfile(model_path):
            raise RuntimeError("model not found: %r" % model_path)
        _CACHE[key] = YoloX(model_path, conf_threshold=conf,
                            nms_threshold=nms, obj_threshold=obj)
    return _CACHE[key]


def main():
    parser = argparse.ArgumentParser(description="Reelcraft YOLOX detector helper")
    parser.add_argument("request")
    parser.add_argument("response")
    parser.add_argument("--model", default=DEFAULT_MODEL)
    parser.add_argument("--confidence", type=float, default=0.35)
    parser.add_argument("--nms", type=float, default=0.5)
    parser.add_argument("--obj", type=float, default=0.5)
    args = parser.parse_args()
    try:
        run(args.request, args.response, args.model, args.confidence, args.nms, args.obj)
    except Exception as exc:  # noqa: BLE001 - report deterministically
        sys.stderr.write("detector helper failed: %s\n" % exc)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
