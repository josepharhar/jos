#ifndef JOS_UNISTD_H_
#define JOS_UNISTD_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef int pid_t;

pid_t fork();
pid_t getpid();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // JOS_UNISTD_H_
