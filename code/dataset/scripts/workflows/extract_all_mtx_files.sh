#!/usr/bin/env bash

set -ueo pipefail

chmod +x tar_extract_mtx_file.sh

for dir in ../../matrices/*/; do
    echo "==============================="
    echo "Extracting mtx files from $dir"
    echo "==============================="
    for tarball in "${dir}"*; do
        echo "==> $tarball"
        ./tar_extract_mtx_file.sh "$tarball"
    done
    echo
done

chmod -x tar_extract_mtx_file.sh
