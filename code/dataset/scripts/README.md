Uses of the following scripts:
+ create_manifest.py - applies filters to the matrices we want to download from SuiteSparse Matrix Collection and writes the manifest.csv file.
+ create_tasks.sh - creates lines of URL and relative path pairs in .tsv.
+ resumable_download.sh - script for having a robust with retry, resumable download, and failure log mechanisms.
+ parallel_downloads.sh - script for 8 parallel downloads using resumable_download.sh
+ create_aria2c_input.py - script for creating an input file for aria2c (if available for robust parallel download)
+ verify_matrices.sh - script for checking integrity of downloaded files.
