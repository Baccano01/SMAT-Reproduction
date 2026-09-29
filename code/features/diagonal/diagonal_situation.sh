#!/usr/bin/env bash

set -euo pipefail

SAMPLES="$1"

# CSV header
echo "Matrix,Ndiags,NTdiags_ratio,ER_DIA"

compute() {

    SAMPLES="$1"

    local exec1="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/diagonal/ndiags/bin/ndiags"
    local exec2="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/diagonal/true_diags/bin/true_diags"

    while read -r line; do

        local out=$("$exec1" "$line")
        if [[ -z $out ]]; then
            continue
        fi

        local diags_count=$(echo "$out" | cut -s -d "," -f 1)
        local er_dia=$(echo "$out" | cut -s -d "," -f 2)

        local tdiags_count=$("$exec2" "$line")
        if [[ -z $tdiags_count ]]; then
            continue
        fi

        local ntdiags_ratio=$(echo "print("$tdiags_count"/"$diags_count")" | python3)

        echo "$line","$diags_count","$ntdiags_ratio","$er_dia"
    done <"$SAMPLES"

}
export -f compute


bash -c "compute $SAMPLES"
