#!/usr/bin/env bash

# set -x to enable access to prompt with PS4 that gets echoed
# at each command run in the script.
# set -m to enable job control inside the script.
set -xmueo pipefail
IFS=$'\n\t'

INPUT_PATH="$1"

# must be absolute paths to avoid ambiguity
diag_script="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/diagonal/diagonal_situation.sh"
md_script="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/matrix_data/parse_matrix_data.sh"
rows_script="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/rows/rows_stat.sh"
powerlaw_script="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/power_law/power_law_exp_factor.py"
join_script="/home/jofeth/University/Master/AdvancedArchitecture/Project/SMAT/code/features/join_csvs.py"

# debugging prompt that gets printed at each command execution.
PS4='+(${BASH_SOURCE}:${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'

# ============================================

echo "Setting up virtual environment and dependencies..."

# setup virtual environment
VENV=".features_ext"
if [[ -z $VENV ]]; then
    echo "Empty VENV variable! :("
fi

# create virtual env
python3 -m venv "$VENV" 2> err.out

# activate virtual env
source "$VENV"/bin/activate 2>> err.out

# install dependencies 
pip3 install -r requirements.txt 2>> err.out



# ============================================

echo "Starting feature extractions..."

# read input state (i.e. the last input CSV file processed).
# important when submitting jobs to SLURM with strict timelimit.
unset START
START=$(cat input_state.txt 2>> err.out)
if [[ -z $START ]]; then
    echo "Failed reading input_state.txt, START var is empty :("
fi

END=32

mkdir -pv "joined"

for ((state=${START} ; state < ${END} ; state++)); do
    INPUT="${INPUT_PATH}/${state}.csv"

    mkdir -pv "raw${state}"
    mkdir -pv "errors${state}"

    # internal set -e, to capture errors in the subshell
    # so the external shell can also capture it with set -e
    bash -c "set -e; python3 ${powerlaw_script} ${INPUT} raw${state}/alphas.csv &> errors${state}/powerlaw_err${state}.out" &
    pid1=$!

    bash -c "set -e; $md_script ${INPUT}" > "raw${state}/mdata${state}.csv"     \
        2> "errors${state}/mdata${state}.out" &
    pid2=$!

    bash -c "set -e; $rows_script ${INPUT}" > "raw${state}/rowdata${state}.csv" \
        2> "errors${state}/rowdata${state}.out" &
    pid3=$!

    bash -c "set -e; $diag_script ${INPUT}" > "raw${state}/dia${state}.csv"     \
        2> "errors${state}/dia_err${state}.out" &
    pid4=$!

    # barrier that waits until all the subshells above terminate
    wait -f $pid1 $pid2 $pid3 $pid4
    
    echo "Joining CSVs of input no. $state..."

    python3 "$join_script" "raw${state}" > "joined/pile${state}.csv"
done

# ============================================
echo "Cleaning up virtual environment..."

# deactivate virtual environment
deactivate

# remove virtual environment
if [[ -n $VENV ]]; then
    rm -rf "$VENV"
fi

echo "Done! :)"



