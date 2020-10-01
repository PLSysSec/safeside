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
set(CLANG ${WASI_SDK_DIR}/bin/clang)
set(CLANG++ ${WASI_SDK_DIR}/bin/clang++)
set(LUCETC ${LUCET_DIR}/target/release/lucetc)
set(WASI_BINDINGS_JSON ${safeside_SOURCE_DIR}/build-lucet/bindings.json)

set(CMAKE_C_COMPILER ${CLANG})
set(CMAKE_CXX_COMPILER ${CLANG++})
set(CMAKE_BUILD_TYPE Release)

# Apply settings suitable for wasm module compilation
set(CMAKE_C_FLAGS
  "--sysroot ${WASI_SYSROOT_DIR} --target=wasm32-wasi")
set(CMAKE_CXX_FLAGS
  "--sysroot ${WASI_SYSROOT_DIR} -fno-exceptions --target=wasm32-wasi")
# Link flags are set by default on Mac - clearing this
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-all")

set(TARGETS spectre_v1_pht_sa ret2spec_sa)
set(TARGETS_SO spectre_v1_pht_sa.so ret2spec_sa.so) # I can't figure out how to automate this in CMake

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(DYLIB_EXT "dylib")
else()
  set(DYLIB_EXT "so")
endif()

FOREACH(TARGET ${TARGETS})

  add_executable(${TARGET}.wasm
    ${safeside_SOURCE_DIR}/demos/cache_sidechannel.cc
    ${safeside_SOURCE_DIR}/demos/instr.cc
    ${safeside_SOURCE_DIR}/demos/timing_array.cc
    ${safeside_SOURCE_DIR}/demos/utils.cc
    #${safeside_SOURCE_DIR}/demos/faults.cc
    ${safeside_SOURCE_DIR}/demos/asm/measurereadlatency_wasm.c
    ${safeside_SOURCE_DIR}/demos/ret2spec_common.cc
    ${safeside_SOURCE_DIR}/demos/${TARGET}.cc)

  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${TARGET}.${DYLIB_EXT}
                     DEPENDS ${TARGET}.wasm
                     COMMAND ${LUCETC}
                             --bindings ${WASI_BINDINGS_JSON}
                             --guard-size "4GiB"
                             --min-reserved-size "4GiB"
                             --max-reserved-size "4GiB"
                             ${TARGET}.wasm
                             -o ${CMAKE_BINARY_DIR}/${TARGET}.${DYLIB_EXT}
                     COMMENT "Compiling wasm file to native")

ENDFOREACH(TARGET)

add_custom_target(all_targets ALL DEPENDS ${TARGETS_SO})

