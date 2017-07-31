#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "list.h"

template <typename T>
class LinkedList : public List<T> {
 public:
  LinkedList() : head_(0) {}
  ~LinkedList() override {
    while (head_) {
      LinkedListEntry<T>* next = head_->next;
      delete head_;
      head_ = next;
    }
  }

  Iterator<T>* GetIterator() override {
    LinkedListIterator<T>* iterator = new LinkedListIterator<T>(head_);
    return iterator;
  }

  void Add(T value) override {
    // LinkedListEntry<T>* new_entry = new LinkedListEntry<T>(std::move(value));
    LinkedListEntry<T>* new_entry = new LinkedListEntry<T>();
    new_entry->value = value;
    new_entry->next = 0;

    if (!head_) {
      head_ = new_entry;
    } else {
      LinkedListEntry<T>* last = head_;
      while (last->next) {
        last = last->next;
      }
      last->next = new_entry;
    }
  }

  bool Remove(T value) override {
    LinkedListEntry<T>* prev = 0;
    LinkedListEntry<T>* entry = head_;
    while (entry) {
      if (entry->value == value) {
        if (prev) {
          prev->next = entry->next;
        }
        delete entry;
        return true;
      } else {
        prev = entry;
        entry = entry->next;
      }
    }
    return false;
  }

 private:
  template <typename E>
  struct LinkedListEntry {
    E value;
    LinkedListEntry<E>* next;
  };

  template <typename E>
  class LinkedListIterator : public Iterator<E> {
   public:
    LinkedListIterator(LinkedListEntry<E>* head) : next_(head) {}
    ~LinkedListIterator() override {}

    E Next() override {
      DCHECK(next_);

      // TODO use fancy std::move() stuff here
      E value = next_->value;
      next_ = next_->next;
      return value;
    }

    bool HasNext() override { return next_; }

   private:
    LinkedListEntry<E>* next_;
  };

  LinkedListEntry<T>* head_;
};

#endif  // LINKED_LIST_H_
