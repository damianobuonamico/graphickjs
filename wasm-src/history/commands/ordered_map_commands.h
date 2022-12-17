#pragma once

#include "../command.h"

#include <unordered_map>
#include <vector>

template <typename K, typename V>
class InsertInOrderedMapCommand: public Command {
  using Map = std::unordered_map<K, V>;
  using Order = std::vector<K>;
  using Pair = std::pair<K, V>;
public:
  InsertInOrderedMapCommand(Map* map, Order* order, const Pair& pair)
    : m_map(map), m_order(order), m_pairs({ pair }), m_indices({ -1 }) {}

  InsertInOrderedMapCommand(Map* map, Order* order, const Pair& pair, int index)
    : m_map(map), m_order(order), m_pairs({ pair }), m_indices({ index }) {}

  virtual void execute() override {
    if (m_map == nullptr || m_order == nullptr) return;

    for (size_t i = 0; i < m_pairs.size(); ++i) {
      Pair& pair = m_pairs[i];
      int index = m_indices[i];

      m_map->insert(pair);

      if (index > -1) {
        m_order->insert(m_order->begin() + index, pair.first);
      } else {
        m_order->push_back(pair.first);
      }
    }
  }

  virtual void undo() override {
    if (m_map == nullptr || m_order == nullptr) return;

    for (int i = static_cast<int>(m_pairs.size()) - 1; i >= 0; --i) {
      Pair& pair = m_pairs[i];
      int index = m_indices[i];

      m_map->erase(pair.first);

      if (index > -1) {
        m_order->erase(m_order->begin() + index);
      } else {
        m_order->erase(std::remove(m_order->begin(), m_order->end(), pair.first), m_order->end());
      }
    }
  }

  virtual bool merge_with(std::unique_ptr<Command>& command) override {
    InsertInOrderedMapCommand* casted_command = dynamic_cast<InsertInOrderedMapCommand*>(command.get());

    if (
      casted_command == nullptr ||
      casted_command->m_map != this->m_map ||
      casted_command->m_order != this->m_order
      ) {
      return false;
    }

    casted_command->m_pairs.insert(casted_command->m_pairs.end(), m_pairs.begin(), m_pairs.end());
    casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

    return true;
  }

  virtual uintptr_t pointer() override {
    return reinterpret_cast<uintptr_t>(m_map);
  }
private:
  Map* m_map;
  Order* m_order;
  std::vector<Pair> m_pairs;
  std::vector<int> m_indices;
};

template <typename K, typename V>
class EraseFromOrderedMapCommand: public Command {
  using Map = std::unordered_map<K, V>;
  using Order = std::vector<K>;
  using Pair = std::pair<K, V>;
public:
  EraseFromOrderedMapCommand(Map* map, Order* order, const Pair& pair)
    : m_map(map), m_order(order), m_pairs({ pair }), m_indices({ }) {
    auto it = std::find(m_order->begin(), m_order->end(), pair.first);

    if (it != m_order->end()) {
      m_indices.push_back(static_cast<int>(it - m_order->begin()));
    } else {
      m_indices.push_back(-1);
    }
  }

  virtual void execute() override {
    if (m_map == nullptr || m_order == nullptr) return;

    for (size_t i = 0; i < m_pairs.size(); ++i) {
      Pair& pair = m_pairs[i];
      int index = m_indices[i];

      m_map->erase(pair.first);

      if (index > -1) {
        m_order->erase(m_order->begin() + index);
      } else {
        m_order->erase(std::remove(m_order->begin(), m_order->end(), pair.first), m_order->end());
      }
    }
  }

  virtual void undo() override {
    if (m_map == nullptr || m_order == nullptr) return;

    for (int i = static_cast<int>(m_pairs.size()) - 1; i >= 0; --i) {
      Pair& pair = m_pairs[i];
      int index = m_indices[i];

      m_map->insert(pair);

      if (index > -1) {
        m_order->insert(m_order->begin() + index, pair.first);
      } else {
        m_order->push_back(pair.first);
      }
    }
  }

  virtual bool merge_with(std::unique_ptr<Command>& command) override {
    EraseFromOrderedMapCommand* casted_command = dynamic_cast<EraseFromOrderedMapCommand*>(command.get());

    if (
      casted_command == nullptr ||
      casted_command->m_map != this->m_map ||
      casted_command->m_order != this->m_order
      ) {
      return false;
    }

    casted_command->m_pairs.insert(casted_command->m_pairs.end(), m_pairs.begin(), m_pairs.end());
    casted_command->m_indices.insert(casted_command->m_indices.end(), m_indices.begin(), m_indices.end());

    return true;
  }

  virtual uintptr_t pointer() override {
    return reinterpret_cast<uintptr_t>(m_map) << 1;
  }
private:
  Map* m_map;
  Order* m_order;
  std::vector<Pair> m_pairs;
  std::vector<int> m_indices;
};
