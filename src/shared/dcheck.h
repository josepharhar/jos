#ifndef DCHECK_H_
#define DCHECK_H_

void DCHECKFailed(const char* condition);
void DCHECKFailedMessage(const char* condition, const char* format, ...);

#define DCHECK(condition)               \
  if (!(condition)) {                   \
    DCHECKFailed(#condition);           \
  }

#define DCHECK_MESSAGE(condition, format, args...)       \
  if (!(condition)) {                                    \
    DCHECKFailedMessage(#condition, format, args);       \
  }

#endif  // DCHECK_H_
