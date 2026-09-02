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
+ rr_distrib_input_csv.py - script for splitting in Round Robin the lines of 'sorted_input_list.csv' in 32 smaller csv files (assuming the lines in the input csv are sorted based on the nnz column, then the csv files produces have the same amount of "low", "medium", and "high" nnz sparse matrices).
+ parallel_proc.sh - jobscript for executing 32 processes to execute SpMV kernel that work on the rows of the 32 smaller csv files produced by 'rr_distrib_input_csv.py'. The script will write on checksum/ and state/ directories the checksums and state (i.e. last row run from the small csv assigned) when the job receives SIG{TERM,INT} from the Slurm scheduler. This makes the script resumable by reading from the corresponding state file in state/. The kernels executed is the Score-P build, the measurement profile will be saved on the directory containing the Matrix Market file pointed by the path read by the script from the smaller csv file.
