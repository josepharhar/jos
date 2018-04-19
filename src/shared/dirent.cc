#include "dirent.h"

DIR* opendir(const char* name) {
  SyscallOpendirParams params;
  params.success_writeback = false;
  Syscall(SYSCALL_OPENDIR, (uint64_t)&params);

  if (!params.success_writeback) {
    return 0;
  }

  DIR* dir = calloc(1, sizeof(DIR));
  return dir;
}

dirent* readdir(DIR* dir) {
}

int closedir(DIR* dir) {
  free(dir);
}
