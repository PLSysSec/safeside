#ifndef DEMOS_INSTR_WASM_H
#define DEMOS_INSTR_WASM_H

#include "compiler_specifics.h"
#include <wasi/api.h>
#include <cstdlib>
#include <cstdio>

inline SAFESIDE_ALWAYS_INLINE void MemoryAndSpeculationBarrier() {
  (void)__wasi_fence();
}

inline void FlushDataCacheLineNoBarrier(const void *address) {
  (void)__wasi_clflush(address);
}

inline void FlushDataCacheLineNoBarrier64(uint64_t address) {
  printf("!!!FlushDataCacheLineNoBarrier64 wasm %llu", reinterpret_cast<unsigned long long>(address));
  abort();
}

#endif  // DEMOS_INSTR_WASM_H
