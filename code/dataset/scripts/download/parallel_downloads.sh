chmod u+x resumable_download.sh
cat tasks.tsv | xargs -P 8 -I {} bash -c 'set -- {}; ./resumable_download.sh "$1" "$2"' _
chmod u-x resumable_download.sh
