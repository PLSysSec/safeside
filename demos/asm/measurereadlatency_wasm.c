#include <time.h>
#include <stdint.h>

uint64_t MeasureReadLatency(const void* address) {
  volatile char* addr = (volatile char*) address;
  clock_t start = clock();
  *addr;
  return clock() - start;
}
