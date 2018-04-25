#include "jstring.h"

namespace stdj {

string::string() {}

string::~string() {
  if (array_) {
    delete[] array_;
  }
}

string::string(const char* other_string) {
  CopyFrom(other_string);
}

string::string(char* other_string) {
  CopyFrom(other_string);
}

string::string(Array<char> array) {
  for (int i = 0; i < array.Size(); i++) {
    Add(array[i]);
  }
  Add(0);
}

void string::Add(char value) {
  // increase array size if needed
  if (size_ + 1 >= array_size_) {
    uint64_t old_array_size = array_size_;
    T* old_array = array_;

    array_size_ = (old_array_size + 5) * 2;
    array_ = new T[array_size_];
    memset(array_, 0, array_size_ * sizeof(T));

    if (old_array) {
      memcpy(array_, old_array, old_array_size * sizeof(T));
      delete[] old_array;
    }
  }

  array_[size_] = value;
  size_++;
}

const char* string::c_str() {
  return Data();
}

void string::CopyFrom(const char* other_string) {
  while (*other_string) {
    Add(*other_string++);
  }
  Add(0);
}

Array<stdj::string> string::Split(string delimiter) {
  Array<stdj::string> output;

  int last_delimiter_index = 0;
  int i = 0;
  while (i < Size() + 1 - delimiter.Size()) {
    string possible_delimiter = Substring(i, i + delimiter.Size());
    if (delimiter == possible_delimiter) {
      if (last_delimiter_index != i) {
        string part = Substring(last_delimiter_index, i);
        output.Add(part);
      }
      i += delimiter.Size();
      last_delimiter_index = i;
    } else {
      i++;
    }
  }

  if (last_delimiter_index != i) {
    string last_part = Substring(last_delimiter_index, i);
    output.Add(last_part);
  }

  return output;
}

void string::CopyFrom(const ArrayBase<T>& other) {
  size_ = other.size_;
  array_size_ = other.array_size_;
  array_ = 0;
  if (array_size_) {
    array_ = new T[array_size_];
    for (int i = 0; i < size_; i++) {
      array_[i] = other.array_[i];
    }
    // memset(array_ + size_, 0, (array_size_ - size_) * sizeof(T));
  }
}

}  // namespace stdj
