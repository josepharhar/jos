#ifndef SEMAPHORE_HANDLER_H_
#define SEMAPHORE_HANDLER_H_

#include "jstring.h"
#include "jmap.h"

struct Semaphore;
// typedef stdj::Map<stdj::string, Semaphore*, ((Semaphore*)0)> SemaphoreMap;
typedef stdj::Map<int, Semaphore*, ((Semaphire*)0)> SemaphoreMap;

void InitSemaphore();

#endif  // SEMAPHORE_HANDLER_H_
