#!/usr/bin/env bash

set -euo pipefail

SAMPLES="$1"
EXEC="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/matrix_data/bin/matrix_data"

echo "Matrix,M,N,NNZ"
while read -r line; do
    echo "$line",$("$EXEC" "$line")
done <"$SAMPLES"
