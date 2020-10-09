#!/bin/bash

if [[ $1 == "pht_sa" ]]; then
  SO=spectre_v1_pht_sa.so
elif [[ $1 == "ret2spec_sa" ]]; then
  SO=ret2spec_sa.so
else
  echo "error: Please specify an argument (either pht_sa or ret2spec_sa)"
  exit
fi

$HOME/spectresandboxing/lucet-spectre/target/release/lucet-wasi \
    --heap-address-space "8GiB" \
    --max-heap-size "4GiB" \
    --stack-size "128MiB" \
    --dir /:/ \
    ./build/$SO
