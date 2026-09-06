#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QMAKE="${QMAKE:-/usr/lib/qt6/bin/qmake}"

cd "$ROOT"
echo "Building application..."
"$QMAKE"
make

cd "$ROOT/tests"
echo "Building tests..."
"$QMAKE"
make

echo "Running tests offscreen..."
# 240s ceiling: Objective 15+ FFmpeg-generated review fixtures raise suite runtime.
QT_QPA_PLATFORM=offscreen timeout 240 ./reelcraft_tests
