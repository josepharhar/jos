#ifndef SHARED_DIRENT_H_
#define SHARED_DIRENT_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct dirent {
  char d_name[256];
};
struct DIR {
  uint64_t id;
};

struct SyscallOpendirParams {
  DIR dir;
  bool success_writeback;
}
struct 

struct DIR* opendir(const char* name);
struct dirent* readdir(DIR* dir);
int closedir(DIR* dir);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // SHARED_DIRENT_H_
