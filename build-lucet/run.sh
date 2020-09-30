#!/bin/bash

$HOME/spectresandboxing/lucet-spectre/target/release/lucet-wasi \
    --heap-address-space "8GiB" \
    --max-heap-size "4GiB" \
    --stack-size "8MiB" \
    --dir /:/ \
    ./build/safeside_lib_lucet.so
