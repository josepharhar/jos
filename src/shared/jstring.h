#ifndef SHARED_JSTRING_H_
#define SHARED_JSTRING_H_

#ifdef TEST
#include <stdio.h>
#include <iostream>
#include <ostream>
#endif

#include "jarray.h"

namespace stdj {

class string {
 public:
  string();

  string(const string& other);
  string& operator=(const string& other);

  string(const char* other_string);
  string(char* other_string);

  void Add(char value);
  int Size() const;
  const char* Data() const;
  const char* c_str() const;
  char Get(int index) const;

  Array<string> Split(string delimiter);
  string Substring(int one, int two);

  string operator+(const string& other);
  char operator[](int index);

  bool Equals(const string& other) const;
  bool operator==(const string& other) const;
  bool operator!=(const string& other) const;

#ifdef TEST
  friend std::ostream& operator<<(std::ostream& os, const string& string) {
    os << "\"" << string.array_ << "\"";
    return os;
  }
#endif

 private:
  char* array_;
  int advertised_size_;
  int actual_size_;

  void AddValueWithSpace(char value);
  int GetRemainingSize();

  void Init();
  void CopyFrom(const char* other_string);
};

}  // namespace stdj

#endif  // SHARED_JSTRING_H_
