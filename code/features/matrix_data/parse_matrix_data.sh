#!/usr/bin/env bash

set -euo pipefail

SAMPLES="$1"

echo "Matrix,M,N,NNZ"
for s in "$SAMPLES"/*; do
    echo "$s",$(bin/matrix_data "$s")
done
