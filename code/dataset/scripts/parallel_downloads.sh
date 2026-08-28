chmod +x resumable_download.sh

tail -n +2 manifest.csv | awk -F',' '{print $4","$2"/"$3".tar.gz"}' | \
    parallel --colsep ',' -j 8 ./resumable_download.sh {1} matrices/{2}
