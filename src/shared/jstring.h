#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#include "jarray.h"

namespace stdj {

class string : public Array<char> {
 public:
  string();
  string(const string& other);
  string& operator=(const string& other);
  string(char* other_string);
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
