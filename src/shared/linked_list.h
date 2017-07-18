#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "list.h"

template <typename T>
class LinkedList : public List<T> {
 public:
  LinkedList() head_(0) {}
  ~LinkedList() override {
    while (head_) {
      LinkedListEntry<T>* next = head_->next;
      delete head_;
      head_ = next;
    }
  }

  Iterator<E> Iterator() override { return LinkedListIterator<T>(head_); }

  bool Add(T value) override {
    LinkedListEntry<T>* new_entry = new LinkedListEntry<T>(std::move(value));

    LinkedListEntry<T>* last = head_;
    while (last->next) {
      last = last->next;
    }
    last->next = new_entry;
  }

  bool Remove(T value) override {}

 private:
  template <typename T>
  class LinkedListEntry<T> {
   public:
    LinkedListEntry(T value) : value_(value), next_(0);

    T value() const { return value_; }

    LinkedListEntry<T>* next() const { return next_; }

    void set_next(LinkedListEntry<T>* next) { next_ = next; }

   private:
    T value_;
    LinkedListEntry<T>* next_;
  };

  template <typename T>
  class LinkedListIterator<T> : public Iterator<T> {
   public:
    LinkedListIterator(LinkedListEntry<T>* head) : next_(head) {}
    ~LinkedListIterator() override {}

    T Next() override {
      DCHECK(next_);

      // TODO use fancy std::move() stuff here
      T entry = next_->entry;
      next_ = next_->next;
      return entry;
    }

    bool HasNext() override { return next_; }

   private:
    LinkedListEntry<T>* next_;
  };

  LinkedListEntry<T>* head_;
};

#endif  // LINKED_LIST_H_
