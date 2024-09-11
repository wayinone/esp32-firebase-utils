#!/bin/sh
set -e

REPO_NAME="esp32-firebase-utils"

# source the current venv
. ./environment/bin/activate

# Install ipykernel and create kernel
python3 -m pip install --no-cache-dir ipykernel jupyterlab
python3 -m ipykernel install --name "$REPO_NAME" --display-name "Python 3 ($REPO_NAME)"
