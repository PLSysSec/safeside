#ifndef DEMOS_INSTR_WASM_H
#define DEMOS_INSTR_WASM_H

#include "compiler_specifics.h"
#include <wasi/api.h>
#include <stdint.h>

inline SAFESIDE_ALWAYS_INLINE void MemoryAndSpeculationBarrier() {
  (void)__wasi_fence();
}

inline void FlushDataCacheLineNoBarrier_GuestAddr(const void *address) {
  (void)__wasi_clflush_guestaddr(address);
}
inline void FlushDataCacheLineNoBarrier_HostAddr(uint64_t address) {
  (void)__wasi_clflush_hostaddr(address);
}

#endif  // DEMOS_INSTR_WASM_H
