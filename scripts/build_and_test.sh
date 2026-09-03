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
QT_QPA_PLATFORM=offscreen timeout 20 ./reelcraft_tests
