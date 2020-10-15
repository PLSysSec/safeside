#!/bin/bash

GDB=gdb

LUCET_WASI=$HOME/spectresandboxing/lucet-spectre/target/release/lucet-wasi
LUCET_WASI_ARGS='--heap-address-space 8GiB --max-heap-size 4GiB --stack-size 8MiB --dir /:/'

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

if [[ $1 == "pht_sa" ]]; then

  SO=spectre_v1_pht_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  --args $LUCET_WASI $LUCET_WASI_ARGS $ASLR_FLAGS ./build/$2/$SO

elif [[ $1 == "ret2spec_sa" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  --args $LUCET_WASI $LUCET_WASI_ARGS $ASLR_FLAGS ./build/$2/$SO

elif [[ $1 == "ret2spec_sa_patched" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS $ASLR_FLAGS ./build/$2/$SO

elif [[ $1 == "ret2spec_sa_patched_nativereturnfalse" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  -x nativereturnfalse.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS $ASLR_FLAGS ./build/$2/$SO

elif [[ $1 == "ret2spec_sa_patched_nativereturntrue" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  -x nativereturntrue.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS $ASLR_FLAGS ./build/$2/$SO

elif [[ $1 == "breakout" ]]; then

  SO=breakout.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x breakout.gdbcommands \
  -ex 'initialize "'$2'"' \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$2/$SO

else
  printf "error: Please specify an argument -- one of:\n  pht_sa\n  ret2spec_sa\n  ret2spec_sa_patched\n  ret2spec_sa_patched_nativereturnfalse\n  ret2spec_sa_patched_nativereturntrue\n  breakout\n"
  exit
fi
