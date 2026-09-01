tail -n +2 manifest.csv | \
    awk -F',' '{print $4"\t""matrices/"$2"/"$3".tar.gz"}' > tasks.tsv
