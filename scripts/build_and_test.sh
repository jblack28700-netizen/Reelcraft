#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QMAKE="${QMAKE:-/usr/lib/qt6/bin/qmake}"

# qmake is only needed to CREATE a Makefile. Makefiles are git-ignored, so a
# fresh checkout has none; once one exists, make regenerates it itself from the
# generated "Makefile:" rule whenever the .pro file or the qmake/mkspec
# configuration behind it changes. Calling qmake unconditionally here therefore
# buys nothing and costs a fixed ~33 s (application) + ~17 s (tests) per run on
# this device -- including runs where nothing changed at all.
ensure_makefile() {
  if [ ! -f Makefile ]; then
    echo "No Makefile present; generating it with qmake..."
    "$QMAKE"
  fi
}

cd "$ROOT"
echo "Building application..."
ensure_makefile
make

cd "$ROOT/tests"
echo "Building tests..."
ensure_makefile
make

echo "Running tests offscreen..."
# 240s ceiling: Objective 15+ FFmpeg-generated review fixtures raise suite runtime.
QT_QPA_PLATFORM=offscreen timeout 240 ./reelcraft_tests
