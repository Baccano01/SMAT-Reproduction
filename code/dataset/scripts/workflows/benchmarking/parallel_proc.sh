#!/usr/bin/env bash
#SBATCH --job-name=bal_csv_run
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=32
#SBATCH --time=00:30:00
#SBATCH --output=job_%j.log

INPUT_CSV="input_list.csv"
BASE_NAME="%{INPUT_CSV%.csv}"
EXEC="../../../spmv/basic_csr/scorep_build"
REPETITIONS=100

# Define the worker function with Score-P path setup, line tracking, and state-saving trap
process_file() {
    local file="$1"
    local executable="$2"
    local reps="$3"
    
    # Extract the directory path leading to the file and configure Score-P
    local file_dir
    file_dir=$(dirname "$file")
    mkdir -p "$file_dir/profile"
    export SCOREP_EXPERIMENT_DIRECTORY="$file_dir/profile"
    
    local index="${file##*_}"
    index="${index%.csv}"

    mkdir -p "checksum"
    mkdir -p "states"
    local checksum_file="checksums/checksum_${index}.csv"
    local state_file="states/last_line_${index}.txt"

    # Read the starting line from the state file if it exists, else default to 0
    local start_line=0
    if [ -f "$state_file" ]; then
        start_line=$(<"$state_file")
    fi

    local line_num=0
    local header_skipped=0
    
    # Trap termination signals to save the last executed line number to state_${index}.csv
    trap 'echo "$line_num" > "state_${index}.csv"; echo "JOB TERMINATED: Stopped at line $line_num in file $file. Saved state to state_${index}.csv" >&2; exit 1' TERM INT

    while IFS= read -r line; do
        if [ "$header_skipped" -eq 0 ]; then
            header_skipped=1
            continue
        fi
        
        ((line_num++))
        
        # Skip lines up to the supplied starting line number
        if [ "$line_num" -le "$start_line" ]; then
            continue
        fi
        
        # Extract the first column
        local mtx_file_path 
        mtx_file_path=$(echo "$line" | awk -F, '{print $1}')
        
        local col_count
        col_count=$(echo "$line" | awk -F, '{print $3}')
        
        # Execute your program
        ./"$executable" "$mtx_file_path" "[1@$col_count]" "$reps"
        
    done < "$file" > "$checksum_file"
}
export -f process_file


# Execute in parallel using xargs (-P 32 for 32 concurrent processes)
# Pass an optional second argument to resume from a specific line 
# (e.g., process_file "{}" 150)
printf '%s\n' "${BASE_NAME}_"*.csv | \
    xargs -I {} -P 32 bash -c 'process_file "{}" "${EXEC}" "${REPETITIONS}"'
