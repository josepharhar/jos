#include "dirent.h"

DIR* opendir(const char* name) {
  SyscallOpendirParams params;
  params.success_writeback = false;
  params.filepath = name;
  Syscall(SYSCALL_OPENDIR, (uint64_t)&params);

  if (!params.success_writeback) {
    return 0;
  }

  DIR* dir = new DIR();
  dir->id = params.id_writeback;
  dir->ents = stdj::Array<dirent*>();
  return dir;
}

dirent* readdir(DIR* dir) {
  SyscallReaddirParams params;
  params.id = dir->id;
  memset(params.filename_writeback, 0, 256);
  params.success_writeback = false;
  params.end_of_files_writeback = false;
  Syscall(SYSCALL_READDIR, (uint64_t)&params);

  if (!params.success_writeback || params.end_of_files_writeback) {
    return 0;
  }

  dirent* ent = new dirent();
  strncpy(ent->d_name, params.filename_writeback, 256);
  dir->ents.Add(ent);
  return ent;
}

int closedir(DIR* dir) {
  for (int i = 0; i < dir->ents.Size(); i++) {
    delete dir->ents.Get(i);
  }

  SyscallClosedirParams params;
  params.id = dir->id;
  params.status_writeback = -1;
  delete dir;
  Syscall(SYSCALL_CLOSEDIR, (uint64_t)&params);

  return params.status_writeback;
}
