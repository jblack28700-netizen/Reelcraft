#!/usr/bin/env python3
"""Reelcraft external speaker/audio helper (speaker provider protocol).

Optional helper behind Reelcraft's replaceable SpeakerEvidenceProvider seam. It
runs Silero VAD (voice activity detection) over a media range and returns
speech intervals.

    Model: Silero VAD (silero_vad.onnx)
    Source: https://github.com/snakers4/silero-vad
    License: MIT (repository and ONNX weights)
    Runtime: Python 3 + ONNX Runtime (MIT); audio decoded with FFmpeg

Silero VAD provides speech *activity*, not speaker identity. This helper reports
a single provider-local speaker id ("spk1"); association to a visible tracked
person is done by Reelcraft (explicit creator binding, direction of arrival
if a provider supplies it, or the single-visible-person rule). A diarization or
audio-visual provider can replace this helper behind the same protocol.

Protocol:
    silero_vad_helper.py <request.json> <response.json> [--model PATH]

request.json:
    { "media": "/path/clip.mp4", "startMs": 114000, "endMs": 126000 }

response.json:
    { "available": true, "provider": "silero-vad", "startMs": 0, "endMs": 12000,
      "intervals": [ { "startMs": 0, "endMs": 2100, "speakerId": "spk1",
                       "confidence": 0.92, "overlap": false } ],
      "error": "" }
"""
import argparse
import json
import os
import subprocess
import sys

import numpy as np

DEFAULT_MODEL = os.environ.get(
    "REELCRAFT_SILERO_MODEL",
    os.path.expanduser("~/.cache/reelcraft/models/silero_vad.onnx"),
)
FFMPEG = os.environ.get("REELCRAFT_FFMPEG", "ffmpeg")
SAMPLE_RATE = 16000
CHUNK = 512
CONTEXT = 64
THRESHOLD = 0.5
MIN_INTERVAL_MS = 100
MERGE_GAP_MS = 200


def decode_audio(media, start_ms, end_ms):
    duration_s = max(0.001, (end_ms - start_ms) / 1000.0)
    args = [FFMPEG, "-v", "error", "-ss", "%.3f" % (start_ms / 1000.0),
            "-t", "%.3f" % duration_s, "-i", media, "-vn", "-ac", "1",
            "-ar", str(SAMPLE_RATE), "-f", "s16le", "-"]
    proc = subprocess.run(args, capture_output=True)
    if proc.returncode != 0 or not proc.stdout:
        raise RuntimeError("no decodable audio stream or ffmpeg failure")
    return np.frombuffer(proc.stdout, dtype=np.int16).astype(np.float32) / 32768.0


def speech_probabilities(audio, model_path):
    import onnxruntime as ort
    if not os.path.isfile(model_path):
        raise RuntimeError("model not found: %r" % model_path)
    session = ort.InferenceSession(model_path, providers=["CPUExecutionProvider"])
    state = np.zeros((2, 1, 128), dtype=np.float32)
    context = np.zeros(CONTEXT, dtype=np.float32)
    probabilities = []
    for i in range(len(audio) // CHUNK):
        chunk = audio[i * CHUNK:(i + 1) * CHUNK]
        x = np.concatenate([context, chunk])[None, :]
        out, state = session.run(
            None, {"input": x, "state": state,
                   "sr": np.array(SAMPLE_RATE, dtype=np.int64)})
        context = x[0, -CONTEXT:]
        probabilities.append(float(out[0, 0]))
    return np.asarray(probabilities)


def intervals_from_probabilities(probabilities):
    intervals = []
    start = None
    for i, probability in enumerate(probabilities):
        t_ms = i * CHUNK * 1000 // SAMPLE_RATE
        if probability >= THRESHOLD and start is None:
            start = t_ms
        elif probability < THRESHOLD and start is not None:
            intervals.append([start, t_ms,
                              float(np.mean(probabilities[
                                  start * SAMPLE_RATE // (CHUNK * 1000):
                                  i]))])
            start = None
    if start is not None:
        intervals.append([start, len(probabilities) * CHUNK * 1000 // SAMPLE_RATE,
                          float(np.mean(probabilities[
                              start * SAMPLE_RATE // (CHUNK * 1000):]))])
    # Merge short gaps and drop very short intervals.
    merged = []
    for interval in intervals:
        if merged and interval[0] - merged[-1][1] <= MERGE_GAP_MS:
            merged[-1][1] = interval[1]
            merged[-1][2] = max(merged[-1][2], interval[2])
        else:
            merged.append(interval)
    return [m for m in merged if m[1] - m[0] >= MIN_INTERVAL_MS]


def run(request_path, response_path, model_path):
    with open(request_path, "r", encoding="utf-8") as fh:
        request = json.load(fh)
    start_ms = int(request.get("startMs", 0))
    end_ms = int(request.get("endMs", 0))
    media = request.get("media", "")
    response = {"available": False, "provider": "silero-vad",
                "startMs": 0, "endMs": max(0, end_ms - start_ms),
                "intervals": [], "error": ""}
    try:
        if not media or not os.path.isfile(media):
            raise RuntimeError("media does not exist: %r" % media)
        audio = decode_audio(media, start_ms, end_ms)
        probabilities = speech_probabilities(audio, model_path)
        raw = intervals_from_probabilities(probabilities)
        response["intervals"] = [
            {"startMs": int(a), "endMs": int(b), "speakerId": "spk1",
             "confidence": round(float(c), 4), "overlap": False}
            for a, b, c in raw
        ]
        response["available"] = True
    except Exception as exc:  # noqa: BLE001 - report deterministically
        response["available"] = False
        response["error"] = str(exc)
    with open(response_path, "w", encoding="utf-8") as fh:
        json.dump(response, fh)


def main():
    parser = argparse.ArgumentParser(description="Reelcraft Silero VAD helper")
    parser.add_argument("request")
    parser.add_argument("response")
    parser.add_argument("--model", default=DEFAULT_MODEL)
    args = parser.parse_args()
    try:
        run(args.request, args.response, args.model)
    except Exception as exc:  # noqa: BLE001
        sys.stderr.write("speaker helper failed: %s\n" % exc)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
