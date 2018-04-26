#ifndef SHARED_DIRENT_H_
#define SHARED_DIRENT_H_

#include "shared/jarray.h"
#include "shared/syscall.h"

struct dirent {
  char d_name[256];
};
struct DIR {
  uint64_t id;
  stdj::Array<dirent*> ents;
};

DIR* opendir(const char* name);
dirent* readdir(DIR* dir);
int closedir(DIR* dir);

#endif  // SHARED_DIRENT_H_
