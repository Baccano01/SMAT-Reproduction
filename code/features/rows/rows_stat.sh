#!/usr/bin/env bash

set -euo pipefail

SAMPLES="$1"
EXEC="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/rows/row_nz/bin/row_stats"

echo "Matrix,max_RD,var_RD,ER_ELL"
while read -r line; do
    "$EXEC" $(realpath "$line")
done <"$SAMPLES"
