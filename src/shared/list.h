#ifndef LIST_H_
#define LIST_H_

template <typename T>
class List {
 public:
  List() : head(0) {}
  ~List() {}

  void Add(T* value) {
    ListEntry<T>* new_entry = new ListEntry<T>();
    new_entry->value = value;
    new_entry->prev = 0;
    new_entry->next = 0;

    if (head) {
      // put new entry before head
      new_entry->prev = head->prev;
      new_entry->next = head;
      head->prev->next = new_entry;
      head->prev = new_entry;
    } else {
      new_entry->prev = new_entry;
      new_entry->next = new_entry;
      head = new_entry;
    }
  }

  void Remove(T* value) {
    ListEntry<T>* entry = GetEntry(value);

    if (entry->next == entry) {
      // removing last entry
      // assert head
      if (entry != head) {
        // TODO add NOTREACHED() and use it here
      }

      delete entry;
      head = 0;
      return;
    }
  
    if (entry == head) {
      head = entry->next;
    }

    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    delete entry;
  }

  T* GetHead() {
    if (head) {
      return head->value;
    } else {
      return 0;
    }
  }

  T* GetPrevious(T* value) {
    ListEntry<T>* entry = GetEntry(value);
    return entry->prev->value;
  }

  T* GetNext(T* value) {
    ListEntry<T>* entry = GetEntry(value);
    return entry->next->value;
  }

  T* GetNextNoLoop(T* value) {
    ListEntry<T>* entry = GetEntry(value)->next;
    if (entry == head) {
      return 0;
    } else {
      return entry->value;
    }
  }

  bool IsEmpty() {
    return head == 0;
  }

  void DestroyAll() {
    while (!IsEmpty()) {
      Remove(GetHead());
    }
  }

 private:
  template <typename E>
  struct ListEntry {
    E* value;
    ListEntry<E>* prev; // may point to self
    ListEntry<E>* next; // may point to self
  };

  ListEntry<T>* head;

  ListEntry<T>* GetEntry(T* value) {
    ListEntry<T>* entry = head;
    do {
      if (entry->value == value) {
        return entry;
      }
      entry = entry->next;
    } while (entry != head);

    return 0;
  }
};

#endif  // LIST_H_
