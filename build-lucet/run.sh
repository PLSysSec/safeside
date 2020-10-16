#!/bin/bash

LUCET_SPECTRE=$HOME/spectresandboxing/lucet-spectre

if [[ $1 == "pht_sa" ]]; then
  SO=spectre_v1_pht_sa.so
elif [[ $1 == "ret2spec_sa" ]]; then
  SO=ret2spec_sa.so
else
  echo "error: First argument should be either pht_sa or ret2spec_sa"
  exit 1
fi

if [[ $2 == "" ]]; then
  echo "error: Requires a second argument, which should be the name of a directory in build/"
  exit 1
elif [ ! -d "./build/$2" ]; then
  echo "error: Second argument should be the name of a directory in build/"
  exit 1
fi

if [[ $2 == *aslr ]]; then
  ASLR_FLAGS="--spectre-mitigation-aslr"
else
  ASLR_FLAGS=
fi

if [[ $2 == cet* ]]; then
  LUCET_WASI=$LUCET_SPECTRE/target-cet/release/lucet-wasi
else
  LUCET_WASI=$LUCET_SPECTRE/target/release/lucet-wasi
fi

$LUCET_WASI \
    --heap-address-space "8GiB" \
    --max-heap-size "4GiB" \
    --stack-size "128MiB" \
    --dir /:/ \
    $ASLR_FLAGS \
    ./build/$2/$SO
