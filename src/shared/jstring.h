#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#include "jarray_base.h"

namespace stdj {

class string : public ArrayBase<char> {
 public:
  string();
  ~string();

  string(const char* other_string);
  string(char* other_string);
  string(Array<char> array);

  void Add(char value) override;

  const char* c_str();
  stdj::Array<stdj::string> Split(stdj::string delimiter);

 protected:
  void CopyFrom(ArrayBase<T>& other) override;
  void CopyFrom(const char* other_string);
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
