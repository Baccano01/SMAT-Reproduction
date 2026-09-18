#!/usr/bin/env bash

set -euo pipefail

SAMPLES="$1"

# CSV header
echo "Matrix,Ndiags,NTdiags_ratio,ER_DIA"

compute() {

    SAMPLES="$1"

    for s in "$SAMPLES"/*; do 

        local out=$(./ndiags/bin/ndiags "$s")
        if [[ -z $out ]]; then
            continue
        fi

        local diags_count=$(echo "$out" | cut -s -d "," -f 1)
        local er_dia=$(echo "$out" | cut -s -d "," -f 2)

        local tdiags_count=$(./true_diags/bin/true_diags "$s")
        if [[ -z $tdiags_count ]]; then
            continue
        fi

        local ntdiags_ratio=$(echo "print("$tdiags_count"/"$diags_count")" | python3)

        echo "$s","$diags_count","$ntdiags_ratio","$er_dia"
    done

}
export -f compute


bash -c "compute $SAMPLES"
