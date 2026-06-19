#!/usr/bin/env bash
set -euo pipefail

TAG=$(git describe --exact-match --tags HEAD 2>/dev/null | sed 's/^v//') || true
HEADER=$(grep 'define CHRONOPLOTTER_VERSION' ChronoPlotter.h | grep -oE '"[0-9]+\.[0-9]+\.[0-9]+"' | tr -d '"')

if [ -z "$TAG" ]; then
    echo "ERROR: HEAD is not at an exact git tag" >&2
    exit 1
fi

if [ "$TAG" != "$HEADER" ]; then
    echo "ERROR: git tag v$TAG does not match ChronoPlotter.h version $HEADER" >&2
    exit 1
fi

echo "OK: git tag v$TAG matches ChronoPlotter.h"
