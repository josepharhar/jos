#ifndef JOS_UNISTD_H_
#define JOS_UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef JOS

typedef int pid_t;

pid_t fork();
pid_t getpid();

void exit();

#endif  // JOS

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_UNISTD_H_
