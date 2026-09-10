#!/usr/bin/env sh
set -eu
RCC="${1:-rcc}"
"$RCC" -binary mod.qrc -o example_modern-1.0.0.rcc
echo "Built: example_modern-1.0.0.rcc"
