#ifndef DCHECK_H_
#define DCHECK_H_

void DCHECKFailed(const char* message);

#define DCHECK(condition)                        \
  if (!(condition)) {                            \
    DCHECKFailed(#condition);                    \
  }

#endif  // DCHECK_H_
