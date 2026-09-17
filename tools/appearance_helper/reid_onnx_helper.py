#!/usr/bin/env python3
"""Reelcraft external appearance/ReID helper (appearance provider protocol).

This is an OPTIONAL helper behind Reelcraft's replaceable AppearanceProvider
seam. It computes a person-appearance embedding for a target crop using a
permissively licensed model:

    OpenVINO Open Model Zoo  person-reidentification-retail-0277
    (ONNX: person-reidentification-retail-0265.onnx, 256-d embedding)
    Code and weights: Apache-2.0 (Open Model Zoo LICENSE)
    Trained on an internal dataset (not Market-1501/MSMT17/DukeMTMC).

Runtime: Python 3 + ONNX Runtime (MIT). Reelcraft's C++ build links neither
Python nor ONNX Runtime; it only speaks the file/JSON protocol.

Protocol:
    reid_onnx_helper.py <request.json> <response.json> [--model PATH]

request.json:
    { "image": "<abs path to PNG crop>", "width": N, "height": N,
      "targetId": "t1", "timeMs": 5500 }

response.json:
    { "embedding": [ ... 256 floats ... ], "quality": 1.0,
      "provider": "reid-retail-0277" }

Model path: REELCRAFT_REID_MODEL or --model.
"""
import argparse
import json
import os
import sys

import cv2 as cv
import numpy as np

DEFAULT_MODEL = os.environ.get(
    "REELCRAFT_REID_MODEL",
    os.path.expanduser("~/.cache/reelcraft/models/reid_0277.onnx"),
)
INPUT_WIDTH = 128
INPUT_HEIGHT = 256

_SESSIONS = {}


def get_session(model_path):
    if model_path not in _SESSIONS:
        import onnxruntime as ort
        if not os.path.isfile(model_path):
            raise RuntimeError("model not found: %r" % model_path)
        # CPU only; a GPU backend can be selected on RunPod later.
        _SESSIONS[model_path] = ort.InferenceSession(
            model_path, providers=["CPUExecutionProvider"])
    return _SESSIONS[model_path]


def encode(model_path, image):
    session = get_session(model_path)
    resized = cv.resize(image, (INPUT_WIDTH, INPUT_HEIGHT),
                        interpolation=cv.INTER_LINEAR)
    # The OMZ model performs its own mean/variance normalization internally; the
    # verified input is raw BGR 0-255, NCHW.
    blob = np.transpose(resized.astype(np.float32), (2, 0, 1))[np.newaxis, :, :, :]
    input_name = session.get_inputs()[0].name
    output = session.run(None, {input_name: blob})[0]
    return np.reshape(output, (-1,)).astype(np.float32)


def run(request_path, response_path, model_path):
    with open(request_path, "r", encoding="utf-8") as fh:
        request = json.load(fh)
    image_path = request.get("image", "")
    if not image_path or not os.path.isfile(image_path):
        raise RuntimeError("request image does not exist: %r" % image_path)
    image = cv.imread(image_path, cv.IMREAD_COLOR)
    if image is None:
        raise RuntimeError("could not decode image: %r" % image_path)

    embedding = encode(model_path, image)
    response = {
        "embedding": [float(v) for v in embedding],
        "dimension": int(embedding.shape[0]),
        "quality": 1.0,
        "provider": "reid-retail-0277",
    }
    with open(response_path, "w", encoding="utf-8") as fh:
        json.dump(response, fh)


def main():
    parser = argparse.ArgumentParser(description="Reelcraft ReID appearance helper")
    parser.add_argument("request")
    parser.add_argument("response")
    parser.add_argument("--model", default=DEFAULT_MODEL)
    args = parser.parse_args()
    try:
        run(args.request, args.response, args.model)
    except Exception as exc:  # noqa: BLE001 - report deterministically
        sys.stderr.write("appearance helper failed: %s\n" % exc)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
