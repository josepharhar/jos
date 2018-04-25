#include "jstring.h"

#include "string.h"

namespace stdj {

const char NULL_STRING[] = "";

string::string() {
  Init();
}

void string::Init() {
  array_ = (char*)NULL_STRING;
  advertised_size_ = 0;
  actual_size_ = 0;
}

string::string(const char* other_string) {
  Init();
  CopyFrom(other_string);
}

string::string(char* other_string) {
  Init();
  CopyFrom(other_string);
}

string::string(const string& other) {
  Init();
  CopyFrom(other.c_str());
}

string& string::operator=(const string& other) {
  Init();
  CopyFrom(other.c_str());
  return *this;
}

void string::Add(char value) {
  // initialize array if needed
  if (!array_ || array_ == NULL_STRING) {
    actual_size_ = 10;
    array_ = new char[actual_size_];
    memset(array_, 0, actual_size_);
    advertised_size_ = 0;
  }

  // increase array size if needed
  if (GetRemainingSize() < 1) {
    // increase array size
    char* old_array = array_;
    int old_actual_size = actual_size_;

    actual_size_ *= 2;
    array_ = new char[actual_size_];
    for (int i = 0; i < old_actual_size; i++) {
      array_[i] = old_array[i];
    }

    delete[] old_array;
  }

  // add the new value
  AddValueWithSpace(value);
}

void string::AddValueWithSpace(char value) {
  DCHECK_MESSAGE(
      GetRemainingSize() > 0,
      "  GetRemainingSize(): %d\n  actual_size_: %d, advertised_size_: %d\n",
      GetRemainingSize(), actual_size_, advertised_size_);

  array_[advertised_size_] = value;
  // add null terminator
  array_[advertised_size_ + 1] = 0;

  advertised_size_++;
}

int string::GetRemainingSize() {
  // account for null terminator
  return (actual_size_ - 1) - advertised_size_;
}

int string::Size() const {
  return advertised_size_;
}

const char* string::Data() const {
  return array_;
}

const char* string::c_str() const {
  return Data();
}

void string::CopyFrom(const char* other_string) {
  while (*other_string) {
    Add(*other_string++);
  }
}

Array<string> string::Split(string delimiter) {
  Array<string> output;

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

string string::Substring(int one, int two) {
  if (one >= two) {
    return string();
  }

  string substring;
  for (int i = one; i < two; i++) {
    substring.Add(array_[i]);
  }
  return substring;
}

string string::operator+(const string& other) {
  string new_string;
  for (int i = 0; i < Size(); i++) {
    new_string.Add(Get(i));
  }
  for (int i = 0; i < other.Size(); i++) {
    new_string.Add(other.Get(i));
  }
  return new_string;
}

char string::Get(int index) const {
  DCHECK(index < advertised_size_);
  return array_[index];
}

char string::operator[](int index) {
  return Get(index);
}

bool string::Equals(const string& other) const {
  return !strcmp(array_, other.array_);
}

bool string::operator==(const string& other) const {
  return Equals(other);
}

bool string::operator!=(const string& other) const {
  return !Equals(other);
}

}  // namespace stdj
