#!/bin/bash

set -eu

readonly RUNS=10000

make condvar-pthread_lib_test-tsan

for ((counter = 0; counter < RUNS; counter++)); do
     echo "Run ${counter}"
     time ./condvar-pthread_lib_test-tsan
     echo ""
done
