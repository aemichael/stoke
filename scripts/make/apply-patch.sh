#!/bin/bash
# Usage: apply-patch.sh <submodule dir> <patch file>
# Applies a patch to a submodule checkout exactly once (idempotent). Uses
# patch(1) rather than git so it also works in copies without git metadata.
set -e
DIR="$1"
PATCH="$(realpath "$2")"
if patch -p1 -R --dry-run -s -f -d "$DIR" < "$PATCH" > /dev/null 2>&1; then
  exit 0  # already applied
fi
echo "Applying $(basename "$PATCH") to $DIR..."
patch -p1 -f -d "$DIR" < "$PATCH"
