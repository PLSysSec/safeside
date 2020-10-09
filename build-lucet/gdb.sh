#!/bin/bash

GDB=gdb

LUCET_WASI=$HOME/spectresandboxing/lucet-spectre/target/release/lucet-wasi
LUCET_WASI_ARGS='--heap-address-space 8GiB --max-heap-size 4GiB --stack-size 8MiB --dir /:/'

NATIVE_RETURNTRUE_ARGS=$(cat <<-END
# replace first instruction of ReturnsTrue(int)_37 with a jump to 

END
) # END must be on its own line, so ) is here

if [[ $1 == "pht_sa" ]]; then

  SO=spectre_v1_pht_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$SO

elif [[ $1 == "ret2spec_sa" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$SO

elif [[ $1 == "ret2spec_sa_patched" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$SO

elif [[ $1 == "ret2spec_sa_patched_nativereturnfalse" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  -x nativereturnfalse.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$SO

elif [[ $1 == "ret2spec_sa_patched_nativereturntrue" ]]; then

  SO=ret2spec_sa.so
  $GDB \
  -ex "set breakpoint pending on" \
  -ex "b guest_func___original_main" \
  -ex "r" \
  -x patch.gdbcommands \
  -x nativereturntrue.gdbcommands \
  --args $LUCET_WASI $LUCET_WASI_ARGS ./build/$SO

else
  printf "error: Please specify an argument -- one of:\n  pht_sa\n  ret2spec_sa\n  ret2spec_sa_patched\n  ret2spec_sa_patched_nativereturnfalse\n  ret2spec_sa_patched_nativereturntrue\n"
  exit
fi
