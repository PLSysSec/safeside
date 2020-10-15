cmake_minimum_required(VERSION 3.13)

# Mac adds extra flags
set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)

project(safeside-wasm
        VERSION 0.1
        DESCRIPTION "Safeside compiled with lucet")

set(safeside_SOURCE_DIR $ENV{HOME}/safeside)
set(LUCET_WASI_DIR $ENV{HOME}/spectresandboxing/lucet-spectre/lucet-wasi)
set(LUCET_DIR $ENV{HOME}/spectresandboxing/lucet-spectre)
set(WASI_SDK_DIR $ENV{HOME}/wasi-sdk/build/install/opt/wasi-sdk)
set(WASI_SYSROOT_DIR ${WASI_SDK_DIR}/share/wasi-sysroot)
set(WASICLANG ${WASI_SDK_DIR}/bin/clang)
set(WASICLANG++ ${WASI_SDK_DIR}/bin/clang++)
set(LUCETC ${LUCET_DIR}/target/release/lucetc)
set(WASI_BINDINGS_JSON ${safeside_SOURCE_DIR}/build-lucet/bindings.json)
set(NATIVE_CLANG++ /usr/bin/clang++)

set(CMAKE_C_COMPILER ${WASICLANG})
set(CMAKE_CXX_COMPILER ${WASICLANG++})
set(CMAKE_BUILD_TYPE Release)

# Apply settings suitable for wasm module compilation
set(CMAKE_C_FLAGS
  "--sysroot ${WASI_SYSROOT_DIR} --target=wasm32-wasi")
set(CMAKE_CXX_FLAGS
  "--sysroot ${WASI_SYSROOT_DIR} -fno-exceptions --target=wasm32-wasi")
# Link flags are set by default on Mac - clearing this
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-all")

set(LUCETC_FLAGS
  "--bindings" "${WASI_BINDINGS_JSON}"
  "--guard-size" "4GiB"
  "--min-reserved-size" "4GiB"
  "--max-reserved-size" "4GiB"
  "--pinned-heap-reg")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(DYLIB_EXT "dylib")
else()
  set(DYLIB_EXT "so")
endif()

set(COMMON_SRCS
  ${safeside_SOURCE_DIR}/demos/cache_sidechannel.cc
  ${safeside_SOURCE_DIR}/demos/instr.cc
  ${safeside_SOURCE_DIR}/demos/timing_array.cc
  ${safeside_SOURCE_DIR}/demos/utils.cc
  #${safeside_SOURCE_DIR}/demos/faults.cc
  ${safeside_SOURCE_DIR}/demos/asm/measurereadlatency_wasm.c
  ${safeside_SOURCE_DIR}/demos/ret2spec_common.cc
)

set(TARGETS
  spectre_v1_pht_sa
  ret2spec_sa
)
set(TARGETS_SO  # I can't figure out how to automate this in CMake
  stock/spectre_v1_pht_sa.${DYLIB_EXT}
  loadlfence/spectre_v1_pht_sa.${DYLIB_EXT}
  strawman/spectre_v1_pht_sa.${DYLIB_EXT}
  sfi_full/spectre_v1_pht_sa.${DYLIB_EXT}
  cet_full/spectre_v1_pht_sa.${DYLIB_EXT}
  sfi_aslr/spectre_v1_pht_sa.${DYLIB_EXT}
  cet_aslr/spectre_v1_pht_sa.${DYLIB_EXT}
  cet_only/spectre_v1_pht_sa.${DYLIB_EXT}
  blade/spectre_v1_pht_sa.${DYLIB_EXT}
  phttobtb/spectre_v1_pht_sa.${DYLIB_EXT}
  interlock/spectre_v1_pht_sa.${DYLIB_EXT}

  stock/ret2spec_sa.${DYLIB_EXT}
  loadlfence/ret2spec_sa.${DYLIB_EXT}
  strawman/ret2spec_sa.${DYLIB_EXT}
  sfi_full/ret2spec_sa.${DYLIB_EXT}
  cet_full/ret2spec_sa.${DYLIB_EXT}
  sfi_aslr/ret2spec_sa.${DYLIB_EXT}
  cet_aslr/ret2spec_sa.${DYLIB_EXT}
  cet_only/ret2spec_sa.${DYLIB_EXT}
  blade/ret2spec_sa.${DYLIB_EXT}
  phttobtb/ret2spec_sa.${DYLIB_EXT}
  interlock/ret2spec_sa.${DYLIB_EXT}
)

FOREACH(TARGET ${TARGETS})

  add_executable(${TARGET}.wasm
    ${COMMON_SRCS}
    ${safeside_SOURCE_DIR}/demos/${TARGET}.cc)

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/stock/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/stock/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with stock protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/loadlfence/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=loadlfence
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/loadlfence/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with loadlfence protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/strawman/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=strawman
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/strawman/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with strawman protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/sfi_full/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=sfi
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/sfi_full/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with sfi_full protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cet_full/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=cet
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/cet_full/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with cet_full protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/sfi_aslr/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=sfiaslr
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/sfi_aslr/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with sfi_aslr protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cet_aslr/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=cetaslr
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/cet_aslr/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with cet_aslr protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cet_only/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-mitigation=cetonly
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/cet_only/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with cet_only protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/blade/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-pht-mitigation=blade
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/blade/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with blade protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/phttobtb/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-pht-mitigation=phttobtb
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/phttobtb/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with phttobtb protections")

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/interlock/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             ${LUCETC_FLAGS}
                             --spectre-pht-mitigation=interlock
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/interlock/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native with interlock protections")

ENDFOREACH(TARGET)

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/ret2spec_sa_native.${DYLIB_EXT}
                   DEPENDS ${safeside_SOURCE_DIR}/demos/ret2spec_common.cc
                           ${safeside_SOURCE_DIR}/demos/cache_sidechannel.cc
                           ${safeside_SOURCE_DIR}/demos/utils.cc
                           ${safeside_SOURCE_DIR}/demos/asm/measurereadlatency_x86_64.S
                           ${safeside_SOURCE_DIR}/demos/cache_sidechannel.h
                           ${safeside_SOURCE_DIR}/demos/local_content.h
                           ${safeside_SOURCE_DIR}/demos/utils.h
                           ${safeside_SOURCE_DIR}/demos/asm/measurereadlatency.h
                           ${safeside_SOURCE_DIR}/demos/ret2spec_sa.cc
                   COMMAND ${NATIVE_CLANG++}
                           -shared -fPIC
                           ${safeside_SOURCE_DIR}/demos/ret2spec_common.cc
                           ${safeside_SOURCE_DIR}/demos/ret2spec_sa.cc
                           ${safeside_SOURCE_DIR}/demos/cache_sidechannel.cc
                           ${safeside_SOURCE_DIR}/demos/utils.cc
                           ${safeside_SOURCE_DIR}/demos/asm/measurereadlatency_x86_64.S
                           -o ${CMAKE_BINARY_DIR}/ret2spec_sa_native.${DYLIB_EXT}
                   COMMENT "Compiling ret2spec_common for native")

add_custom_target(all_targets ALL DEPENDS ${TARGETS_SO} ret2spec_sa_native.${DYLIB_EXT})

