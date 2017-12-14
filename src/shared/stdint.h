// For some reason, the regular stdint.h doesn't work
#ifndef STDINT_H_
#define STDINT_H_

// test programs already have this defined from the actual c++ stdlib
#ifndef TEST

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long int64_t;
typedef unsigned long uint64_t;

#endif  // TEST

#endif  // STDINT_H_
