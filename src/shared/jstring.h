#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#include "jarray.h"

namespace stdj {

class string : public Array<char> {
 public:
  string(char* other_string);
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
