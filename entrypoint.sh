#!/usr/bin/env bash
set -euo pipefail

cd "$WORKSPACE"

git config --global user.email ""
git config --global user.name ""

# Initialize the repo workspace on first run (e.g. when /workspace is a
# fresh bind mount without an existing .repo directory).
if [ ! -d "$WORKSPACE/.repo" ]; then
    echo "Initializing repo workspace at $WORKSPACE..."
    repo init -u "$MANIFEST_URL" -b "$MANIFEST_BRANCH" -m default-edf.xml
fi

repo sync

BUILD_DIR="${BUILD_DIR:-build-small}"

# Automatically source the Yocto/Xilinx SDK setup so bitbake is ready to use
# as soon as the container starts, matching the manual steps in README.md.
if [ -f "$WORKSPACE/edf-init-build-env" ]; then
    # setupsdk probes $ZSH_NAME to detect the running shell, which is unset
    # under bash; temporarily relax -u so that doesn't abort the script.
    set +u
    # shellcheck disable=SC1090
    source edf-init-build-env "$BUILD_DIR"
    set -u
else
    echo "Warning: $WORKSPACE/edf-init-build-env not found; skipping automatic environment setup." >&2
fi

exec "$@"
