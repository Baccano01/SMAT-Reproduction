### downloads/
desc: scripts for downloading the tarballs of sparse matrices from https://sparse.tamu.edu in Matrix Market format. Check each script for USAGE details.
+ create_manifest.py - applies filters to the matrices we want to download from SuiteSparse Matrix Collection and writes the manifest.csv file.
+ create_tasks.sh - creates lines of URL and relative path pairs in .tsv.
+ resumable_download.sh - script for having a robust with retry, resumable download, and failure log mechanisms.
+ parallel_downloads.sh - script for 8 parallel downloads using resumable_download.sh
+ create_aria2c_input.py - script for creating an input file for aria2c (if available for robust parallel download)

### verif/:
desc: scripts for checking integrity of downloaded tarballs. Check each script for USAGE details.
+ verify_matrices.sh - script for checking integrity of downloaded files.

### workflow/:
desc: scripts and binaries for preprocessing downloaded tarballs. Check each script and eventual README for USAGE details.
+ mtx_mrkt_dims - contains executable for printing the relative path of the Matrix Market file, row count, col count, and number of nonzeros.
+ tar_extract_mtx_file.sh - script for extracing the .mtx file from a tarball file (.tar.gz)
+ extract_all_mtx_files.sh - driver script of tar_extract_mtx_file.sh to be used on the parent directory of the subdirectories that contain the tarballs.
+ create_input_list.sh - script for creating input_list.csv that contains comma separated lines of 'mtx_filepath,row_count,col_count,nnz' from each Matrix Market (.mtx) file applied on the parent directory of the subdirectories that contain the tarballs.
