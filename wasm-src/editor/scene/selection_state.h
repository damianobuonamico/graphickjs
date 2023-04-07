#pragma once

#include "entity.h"
#include "../../math/box.h"
#include "../../utils/pointers.h"

#include <unordered_map>

class SelectionState {
public:
  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const UUID, Entity*>;
    using reference = std::pair<const UUID, Entity*>&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;

    iterator(
      typename std::unordered_map<UUID, Entity*>::iterator first_iterator,
      std::unordered_map<UUID, Entity*>::iterator first_end_iterator,
      std::unordered_map<UUID, Entity*>::iterator second_begin_iterator,
      std::unordered_map<UUID, Entity*>::iterator second_iterator
    );

    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

    reference operator*();
    pointer operator->();
  private:
    std::unordered_map<UUID, Entity*>::iterator m_first_iterator;
    std::unordered_map<UUID, Entity*>::iterator m_first_end_iterator;
    std::unordered_map<UUID, Entity*>::iterator m_second_begin_iterator;
    std::unordered_map<UUID, Entity*>::iterator m_second_iterator;
  };

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<const UUID, const Entity*>;
    using reference = const std::pair<const UUID&, const Entity*>;
    using pointer = DataPointer<value_type>;
    using difference_type = std::ptrdiff_t;

    const_iterator(
      std::unordered_map<UUID, Entity*>::const_iterator first_iterator,
      std::unordered_map<UUID, Entity*>::const_iterator first_end_iterator,
      std::unordered_map<UUID, Entity*>::const_iterator second_begin_iterator,
      std::unordered_map<UUID, Entity*>::const_iterator second_iterator
    );

    const_iterator& operator++();
    const_iterator operator++(int);

    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;

    reference operator*();
    pointer operator->();
  private:
    std::unordered_map<UUID, Entity*>::const_iterator m_first_iterator;
    std::unordered_map<UUID, Entity*>::const_iterator m_first_end_iterator;
    std::unordered_map<UUID, Entity*>::const_iterator m_second_begin_iterator;
    std::unordered_map<UUID, Entity*>::const_iterator m_second_iterator;
  };
public:
  SelectionState(const SelectionState&) = default;
  SelectionState(SelectionState&&) = default;

  ~SelectionState() = default;

  inline iterator begin() {
    return {
      m_selected.begin(), m_selected.end(),
      m_temp_selected.begin(),m_temp_selected.begin()
    };
  }
  inline iterator end() {
    return {
      m_selected.end(), m_selected.end(),
      m_temp_selected.begin(), m_temp_selected.end()
    };
  }
  inline const_iterator begin() const {
    return {
      m_selected.begin(), m_selected.end(),
      m_temp_selected.begin(),m_temp_selected.begin()
    };
  }
  inline const_iterator end() const {
    return {
      m_selected.end(), m_selected.end(),
      m_temp_selected.begin(), m_temp_selected.end()
    };
  }

  inline size_t size() const { return m_selected.size() + m_temp_selected.size(); }
  inline bool empty() const { return size() < 1; }
  inline bool has(UUID id) const {
    return m_selected.find(id) != m_selected.end() ||
      m_temp_selected.find(id) != m_temp_selected.end();
  }

  std::vector<Entity*> entities();
  Box bounding_box() const;

  void clear();
  void select(Entity* entity, bool select_children = true);
  void deselect(UUID id, bool deselect_children = true);
  void temp_select(std::vector<Entity*> entities);
  void sync(bool sync_children = false);
private:
  SelectionState() = default;
private:
  std::unordered_map<UUID, Entity*> m_selected;
  std::unordered_map<UUID, Entity*> m_temp_selected;
private:
  friend class Scene;
};
