#!/usr/env/bin bash

set -ueo pipefail

./tester ../mtx_mrkt_csr/test/samples/ mm 1> test.out 2> errors.out

./tester ../mtx_mrkt_csr/test/samples/ mtx 1>> test.out 2>> errors.out

N_TESTS=$(grep -F "+" test.out --count)
N_OKS=$(grep -F "ok" test.out --count)
echo "== #tests: $N_TESTS =="
echo "== #oks: $N_OKS =="
