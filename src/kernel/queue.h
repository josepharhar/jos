#ifndef QUEUE_H_
#define QUEUE_H_

#include "kmalloc.h"

template <typename T>
struct QueueEntry {
  T* value;
  QueueEntry<T>* next;
};

template <typename T>
class Queue {
 public:
  Queue() : head(0) {}
  ~Queue() {}

  void Add(T* value) {
    QueueEntry<T>* new_entry = (QueueEntry<T>*) kmalloc(sizeof(QueueEntry<T>));
    new_entry->value = value;
    new_entry->next = 0;

    if (head) {
      GetLastEntry()->next = new_entry;
    } else {
      head = new_entry;
    }
  }

  T* Remove() {
    if (!head) {
      return 0;
    }

    T* removed_value = head->value;

    QueueEntry<T>* head_to_remove = head;
    head = head->next;
    kfree(head_to_remove);

    return removed_value;
  }


  T* Peek() {
    if (head) {
      return head->value;
    } else {
      return 0;
    }
  }

  bool IsEmpty() {
    return head == 0;
  }

 private:
  QueueEntry<T>* GetLastEntry() {
    if (!head) {
      return 0;
    }

    QueueEntry<T>* last_entry = head;
    while (last_entry->next) {
      last_entry = last_entry->next;
    }
    return last_entry;
  }

  QueueEntry<T>* head; // head is the front of the queue
};

#endif  // QUEUE_H_
