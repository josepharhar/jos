#ifndef REFCOUNTED_H_
#define REFCOUNTED_H_

// TODO write userspace test for this
template <typename T>
class RefCounted {
 public:
  RefCounted(T* pointer) {
    refcount_ = new uint64_t;
    *refcount_ = 1;
  }
  RefCounted(const RefCounted& other) {
    refcount_ = other.refcount_;
    pointer_ = other.pointer_;
    *refcount_++;
  }
  RefCounted& operator=(const RefCounted& other) {
    refcount_ = other.refcount_;
    pointer_ = other.pointer_;
    *refcount_++;
  }

  ~RefCounted() {
    *refcount_--;
    if (*refcount_ < 1) {
      delete refcount_;
      delete pointer_;
    }
  }

  T* get() {
    return pointer_;
  }
  /*T operator*() {
    return *get();
  }*/
  T* operator->() {
    return get();
  }

 private:
  T* pointer_;
  uint64_t* refcount_;
};

#endif  // REFCOUNTED_H_
