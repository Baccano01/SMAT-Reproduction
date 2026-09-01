#!/usr/bin/env bash

# -e: automatically exits the script immediately if any standalone commands fail.
# -u: exits script if it tries to use an unset variable.
# -o pipefail: failures inside pipelines also trigger an immediate exit.
set -ueo pipefail

TARBALL="$1"

# extract the basename from TARBALL
name=$(basename "$TARBALL" .tar.gz)

mtx_file="$name/$name.mtx"
parent=$(dirname "$TARBALL")

tar --directory="$parent" -xzf "$TARBALL" "$mtx_file" 
echo "=>Extracted: $mtx_file"
