#ifndef SHARED_UMALLOC_H_
#define SHARED_UMALLOC_H_

void* umalloc(int size);
void ufree(void* ptr);
void* ucalloc(int size);

#endif  // SHARED_UMALLOC_H_
