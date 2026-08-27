#!/usr/env/bash

set -ueo pipefail

start_time=$(date +%s)
echo $(date)
./unit_test test/samples/Fashion_MNIST_norm_10NN.mtx "[1@10000]" > stress_test.out
end_time=$(date +%s)
walltime=$(( end_time-start_time ))
echo "walltime (s) is : $walltime"
