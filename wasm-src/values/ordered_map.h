#pragma once

#include "../history/command_history.h"
#include "../history/commands/ordered_map_commands.h"
#include "../utils/pointers.h"

#include <unordered_map>
#include <stack>

template <typename K, typename V>
class OrderedMap {
  using Map = std::unordered_map<K, V>;
  using Order = std::vector<K>;
public:
  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const K, V>;
    using reference = std::pair<const K&, V&>;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;

    iterator(typename std::vector<K>::iterator key_iterator,
      std::unordered_map<K, V>& map)
      : m_key_iterator(key_iterator), m_map(map) {}

    iterator& operator++() {
      ++m_key_iterator;
      return *this;
    }

    iterator operator++(int) {
      iterator tmp(*this);
      ++m_key_iterator;
      return tmp;
    }

    iterator& operator--() {
      --m_key_iterator;
      return *this;
    }

    iterator operator--(int) {
      iterator tmp(*this);
      --m_key_iterator;
      return tmp;
    }

    bool operator==(const iterator& other) const {
      return m_key_iterator == other.m_key_iterator;
    }

    bool operator!=(const iterator& other) const {
      return m_key_iterator != other.m_key_iterator;
    }

    reference operator*() {
      return { *m_key_iterator, m_map.at(*m_key_iterator) };
    }

    pointer operator->() {
      return &(operator*());
    }

  private:
    typename std::vector<K>::iterator m_key_iterator;
    std::unordered_map<K, V>& m_map;
  };

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const K&, const V&>;
    using reference = std::pair<const K&, const V&>;
    using pointer = DataPointer<value_type>;
    using difference_type = std::ptrdiff_t;

    const_iterator(typename std::vector<K>::const_iterator key_iterator,
      const std::unordered_map<K, V>& map)
      : m_key_iterator(key_iterator), m_map(map) {}

    const_iterator& operator++() {
      ++m_key_iterator;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp(*this);
      ++m_key_iterator;
      return tmp;
    }

    const_iterator& operator--() {
      --m_key_iterator;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator tmp(*this);
      --m_key_iterator;
      return tmp;
    }

    bool operator==(const const_iterator& other) const {
      return m_key_iterator == other.m_key_iterator;
    }

    bool operator!=(const const_iterator& other) const {
      return m_key_iterator != other.m_key_iterator;
    }

    reference operator*() const {
      return { *m_key_iterator, m_map.at(*m_key_iterator) };
    }

    pointer operator->() {
      return { operator*() };
    }
  private:
    typename std::vector<K>::const_iterator m_key_iterator;
    const std::unordered_map<K, V>& m_map;
  };
public:
  inline iterator begin() { return { m_order.begin(), m_map }; }
  inline iterator end() { return { m_order.end(), m_map }; }
  inline const_iterator begin() const { return { m_order.begin(), m_map }; }
  inline const_iterator end() const { return { m_order.end(), m_map }; }
  inline size_t size() const { return m_map.size(); }

  void insert(const std::pair<K, V>& element) {
    CommandHistory::add(std::make_unique<InsertInOrderedMapCommand<K, V>>(&m_map, &m_order, element));
  }

  void insert(const std::pair<K, V>& element, int index) {
    CommandHistory::add(std::make_unique<InsertInOrderedMapCommand<K, V>>(&m_map, &m_order, element, index));
  }

  void erase(const K& key) {
    auto it = m_map.find(key);
    if (it == m_map.end()) return;

    CommandHistory::add(std::make_unique<EraseFromOrderedMapCommand<K, V>>(&m_map, &m_order, *it));
  }
private:
  Map m_map;
  Order m_order;
};
