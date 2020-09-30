#ifndef DEMOS_INSTR_WASM_H
#define DEMOS_INSTR_WASM_H

#include "compiler_specifics.h"
#include <wasi/api.h>

inline SAFESIDE_ALWAYS_INLINE void MemoryAndSpeculationBarrier() {
  (void)__wasi_fence();
}

inline void FlushDataCacheLineNoBarrier(const void *address) {
  (void)__wasi_clflush(address);
}

#endif  // DEMOS_INSTR_WASM_H
