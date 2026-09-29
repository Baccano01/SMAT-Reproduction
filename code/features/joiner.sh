#!/usr/bin/env bash

set -euo pipefail

for i in {0..31}; do 
    uv run join_csvs.py "raw${i}" > "joined/pile${i}.csv"
done
