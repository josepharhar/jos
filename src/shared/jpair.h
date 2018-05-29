#ifndef SHARED_JPAIR_H_
#define SHARED_JPAIR_H_

namespace stdj {

template <typename T1, typename T2>
class pair {
 public:
  pair() {}
  pair(T1 new_first, T2 new_second) : first(new_first), second(new_second) {}

  T1 first;
  T2 second;
};

}  // namespace stdj

#endif  // SHARED_JPAIR_H_
