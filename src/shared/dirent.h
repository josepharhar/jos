#ifndef SHARED_DIRENT_H_
#define SHARED_DIRENT_H_

#include "shared/jarray.h"

struct dirent {
  char d_name[256];
};
struct DIR {
  uint64_t id;
  stdj::Array<dirent*> ents;
};

struct SyscallOpendirParams {
  const char* filepath;
  uint64_t id_writeback;
  bool success_writeback;
}
struct SyscallReaddirParams {
  uint64_t id;
  char filename_writeback[256];
  bool success_writeback;
}
struct SyscallClosedirParams {
  uint64_t id;
  int status_writeback;
}

DIR* opendir(const char* name);
dirent* readdir(DIR* dir);
int closedir(DIR* dir);

#endif  // SHARED_DIRENT_H_
