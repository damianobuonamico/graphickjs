#pragma once

#include "../command_history.h"
#include "../commands/vector_commands.h"

#include <vector>
#include <utility>

namespace Graphick::History {

  template <typename K, typename V>
  class MapValue {
  public:
    using value = std::pair<K, V>;
    using vector = std::vector<value>;
    using iterator = typename vector::iterator;
    using const_iterator = typename vector::const_iterator;
  public:
    MapValue(std::initializer_list<value> list) : m_vector(list) {}
    MapValue(const vector& vector) : m_vector(vector) {}
    MapValue() {}

    inline iterator begin() { return m_vector.begin(); }
    inline iterator end() { return m_vector.end(); }
    inline const_iterator begin() const { return m_vector.begin(); }
    inline const_iterator end() const { return m_vector.end(); }

    inline std::reverse_iterator<iterator> rbegin() { return m_vector.rbegin(); }
    inline std::reverse_iterator<iterator> rend() { return m_vector.rend(); }
    inline std::reverse_iterator<const_iterator> rbegin() const { return m_vector.rbegin(); }
    inline std::reverse_iterator<const_iterator> rend() const { return m_vector.rend(); }

    inline size_t size() const { return m_vector.size(); }

    inline void insert(const value& pair) {
      CommandHistory::add(std::make_unique<InsertInVectorCommand<value>>(&m_vector, pair));
    }

    inline void insert(const value& pair, int index) {
      CommandHistory::add(std::make_unique<InsertInVectorCommand<value>>(&m_vector, pair, index));
    }

    // TODO: Implement
    inline void erase(const value& pair) {
      CommandHistory::add(std::make_unique<EraseFromVectorCommand<value>>(&m_vector, pair));
    }

    inline void erase(const value& pair, int index) {
      CommandHistory::add(std::make_unique<EraseFromVectorCommand<value>>(&m_vector, pair, index));
    }
  private:
    vector m_vector;
  };

}
