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
# 900s ceiling. The complete suite (446 tests, including the real-FFmpeg render,
# decode and analysis tests added since Objective 19) measures ~720-743 s on this
# proot device, so the old 240 s value -- written when the suite ran in ~221-241 s
# -- killed it partway through and made the script exit 124. 900 s leaves ~21%
# headroom over the slowest observed run. If this ceiling is ever reached, measure
# the suite again before raising it further.
QT_QPA_PLATFORM=offscreen timeout 900 ./reelcraft_tests
