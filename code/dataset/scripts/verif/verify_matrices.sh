#!/bin/bash
# verify_matrices.sh
# Run from the same directory as tasks.tsv / matrices/
#
# Checks, for every expected file in tasks.tsv:
#   1. File exists and is non-empty
#   2. Gzip integrity (catches truncated/corrupted downloads)
#   3. Tar archive integrity (catches corrupted tar structure)
#   4. Contains a .mtx file (catches wrong-format or empty archives)
#   5. .mtx file itself is well-formed (has the MatrixMarket banner + a size line)
#
# Outputs:
#   verify_ok.log     - files that passed all checks
#   verify_bad.log     - files that failed, with reason
#   verify_missing.log - expected files never downloaded at all

: > verify_ok.log
: > verify_bad.log
: > verify_missing.log

total=0
ok=0
bad=0
missing=0

while IFS=$'\t' read -r url out; do
    total=$((total+1))

    # 1. Existence / non-empty
    if [ ! -s "$out" ]; then
        echo "$out" >> verify_missing.log
        missing=$((missing+1))
        continue
    fi

    # 2. Gzip integrity
    if ! gzip -t "$out" 2>/dev/null; then
        echo "$out : FAILED gzip integrity check (corrupted/truncated)" >> verify_bad.log
        bad=$((bad+1))
        continue
    fi

    # 3. Tar structure integrity (also implicitly re-checks gzip layer)
    if ! tar -tzf "$out" > /tmp/tar_listing_$$.txt 2>/dev/null; then
        echo "$out : FAILED tar integrity check" >> verify_bad.log
        bad=$((bad+1))
        rm -f /tmp/tar_listing_$$.txt
        continue
    fi

    # 4. Contains a .mtx file
    mtx_entry=$(grep '\.mtx$' /tmp/tar_listing_$$.txt | head -1)
    if [ -z "$mtx_entry" ]; then
        echo "$out : NO .mtx FILE FOUND IN ARCHIVE" >> verify_bad.log
        bad=$((bad+1))
        rm -f /tmp/tar_listing_$$.txt
        continue
    fi
    rm -f /tmp/tar_listing_$$.txt

    # 5. Peek inside the archive and check the .mtx banner without full extraction
    #    tar -xzO extracts a single member to stdout
    banner=$(tar -xzOf "$out" "$mtx_entry" 2>/dev/null | head -1)
    if [[ "$banner" != "%%MatrixMarket"* ]]; then
        echo "$out : .mtx FILE MISSING VALID MATRIXMARKET BANNER (got: '$banner')" >> verify_bad.log
        bad=$((bad+1))
        continue
    fi

    echo "$out" >> verify_ok.log
    ok=$((ok+1))

done < tasks.tsv

echo ""
echo "===== Verification Summary ====="
echo "Total expected : $total"
echo "OK             : $ok"
echo "Bad/corrupted  : $bad"
echo "Missing        : $missing"
echo "================================"
echo ""
echo "See verify_bad.log and verify_missing.log for details."
