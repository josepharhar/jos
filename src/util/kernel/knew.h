#ifndef UTIL_KNEW_H_
#define UTIL_KNEW_H_

void* operator new(uint64_t size);
void operator delete(void* address);
void operator delete(void* address, uint64_t size);

#endif  // UTIL_KNEW_H_
