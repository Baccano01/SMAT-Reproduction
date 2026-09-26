#!/usr/bin/env bash

set -euo pipefail

SAMPLE="$1"

echo "Matrix,max_RD,var_RD,ER_ELL" > test.csv 2> err.out
for s in "$SAMPLE"/*; do
    ../bin/row_stats $(realpath "$s") >> test.csv 2>> err.out
done
