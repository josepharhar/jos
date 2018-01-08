#ifndef JMAP_H_
#define JMAP_H_

namespace stdj {

template <typename K, typename V, V NullV>
class Map {
 public:
  Map() : head_(0) {}
  ~Map() {
    Entry* next = 0;
    while (head_) {
      next = head_->next;
      delete head_;
      head_ = next;
    }
  }

  Map(const Map& other) {
    // TODO test this
    head_ = 0;
    if (!other.head_) {
      return;
    }
    head_ = new Entry(*other.head_);
    Entry* entry = head_;
    while (entry->next) {
      entry->next = new Entry(*entry->next);
      entry = entry->next;
    }
  }
  Map& operator=(const Map& other) = delete;

  Map(Map&& other) = delete;
  Map& operator=(Map&& other) = delete;

  void Set(K key, V value) {
    // check for duplicates first
    Entry* entry = head_;
    while (entry) {
      if (entry->key == key) {
        entry->value = value;
        return;
      }
      entry = entry->next;
    }

    // no duplicates, create a new entry.
    Entry* new_entry = new Entry();
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = head_;
    head_ = new_entry;
  }

  V Get(K key) const {
    Entry* current_entry = head_;
    while (current_entry) {
      if (current_entry->key == key) {
        return current_entry->value;
      }
      current_entry = current_entry->next;
    }

    // Could not find the key.
    return NullV;
  }

  K GetKeyAt(int index) const {
    Entry* entry = GetEntryAt(index);
    // TODO error checking, entry can be null but there is no NullV to return.
    return entry->key;
  }

  V GetAt(int index) const {
    Entry* entry = GetEntryAt(index);
    return entry ? entry->value : NullV;
  }

  bool ContainsKey(K key) const {
    Entry* current_entry = head_;
    while (current_entry) {
      if (current_entry->key == key) {
        return true;
      }
      current_entry = current_entry->next;
    }
    return false;
  }

  // Removes ALL entries in the map with the corresponding key
  void Remove(K key) {
    while (head_ && head_->key == key) {
      Entry* old_head = head_;
      head_ = old_head->next;
      delete old_head;
    }

    Entry* current_entry = head_;
    Entry* next_entry = current_entry ? current_entry->next : 0;
    while (current_entry && next_entry) {
      if (next_entry->key == key) {
        current_entry->next = next_entry->next;
        delete next_entry;
      }

      current_entry = current_entry->next;
      next_entry = current_entry ? current_entry->next : 0;
    }
  }

  int Size() const {
    int size = 0;
    Entry* entry = head_;
    while (entry) {
      entry = entry->next;
      size++;
    }
    return size;
  }

 private:
  struct Entry {
    K key;
    V value;
    Entry* next;
  };

  Entry* GetEntryAt(int index) const {
    int current_index = 0;
    Entry* current_entry = head_;
    while (current_entry) {
      if (current_index == index) {
        return current_entry;
      }
      current_entry = current_entry->next;
      current_index++;
    }
    return 0;
  }

  Entry* head_;
};

}  // namespace stdj

#endif  // JMAP_H_
