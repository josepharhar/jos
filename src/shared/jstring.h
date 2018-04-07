#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#include "jarray.h"

namespace stdj {

class string : public Array<char> {
 public:
  string();
  string(const string& other);
  string& operator=(const string& other);
  string(const char* other_string);
  string(Array<char> array);

  const char* c_str();
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
