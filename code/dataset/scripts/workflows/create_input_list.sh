#!/usr/bin/env bash

set -ueo pipefail

echo "============================="
echo "Creating input list..."
echo "============================="

echo "mtx_filepath,row_count,col_count,nnz" > input_list.csv

for dir in ../../matrices/*/; do
    echo "==============================="
    echo "processing $dir"
    echo "==============================="
    for subdir in "${dir}"*/; do
        echo "==> $subdir"
        ./print_pathNdims "${subdir}"/*.mtx >> input_list.csv
    done
    echo
done

echo "============================="
echo "input list created... :)"
echo "============================="
