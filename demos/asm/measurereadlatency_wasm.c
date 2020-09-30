#include <wasi/api.h>
#include <stdint.h>
#include <stdio.h>

uint64_t MeasureReadLatency(const void* address) {
  volatile char* addr = (volatile char*) address;
  uint64_t start, end;
  (void)__wasi_rdtsc(&start);
  *addr;
  (void)__wasi_rdtsc(&end);
  return end - start;
}
