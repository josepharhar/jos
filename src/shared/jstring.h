#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#include "jarray.h"

namespace stdj {

class string : public Array<char> {
 public:
  string();

  string(const char* other_string);
  string(char* other_string);
  string(Array<char> array);

  const char* c_str();

 private:
  void CopyFrom(const char* other_string);
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
