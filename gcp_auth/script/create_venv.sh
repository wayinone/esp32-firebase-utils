#!/bin/sh


if [ ! -e "environment/bin/activate" ]; then
  echo "venv not exists"
  python3.10 -m venv ./environment
fi

. ./environment/bin/activate

python -m pip install --no-cache-dir  pip setuptools pypandoc pip-tools
python -m pip install -r ./src/requirements.txt
python -m pip install .
