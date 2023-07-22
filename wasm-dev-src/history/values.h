#pragma once

#include "command_history.h"
#include "commands.h"

#include "../math/vec2.h"
#include "../math/vector.h"

#include <memory>

namespace Graphick::History {

  class BoolValue {
  public:
    BoolValue() : m_value(false) {};
    BoolValue(const bool value) : m_value(value) {};

    inline bool get() const { return m_value; };

    void set(const bool value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<bool>>(m_value, value));
    };
  private:
    bool m_value;
  };

  class FloatValue {
  public:
    FloatValue() : m_value(0.0f) {};
    FloatValue(const float value) : m_value(value) {};

    inline float get() const { return m_value + m_delta; };
    inline float delta() const { return m_delta; };

    void set(const float value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, value));
      m_delta = 0.0f;
    };

    void add(const float amount) {
      if (amount == 0.0f) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, m_value + amount));
    };

    void set_delta(const float value) {
      m_delta = value;
    };

    void add_delta(const float amount) {
      m_delta += amount;
    };

    void move_to(const float value) {
      float delta = value - get();
      m_delta += delta;
    };

    void apply() {
      if (m_delta == 0.0f) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<float>>(m_value, get()));
      m_delta = 0.0f;
    };
  private:
    float m_value;
    float m_delta = 0.0f;
  };

  class IntValue {
  public:
    IntValue() : m_value(0) {};
    IntValue(const int value) : m_value(value) {};

    inline int get() const { return m_value + m_delta; };
    inline int delta() const { return m_delta; };

    void set(const int value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, value));
      m_delta = 0;
    };

    void add(const int amount) {
      if (amount == 0) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, m_value + amount));
    };

    void set_delta(const int value) {
      m_delta = value;
    };

    void add_delta(const int amount) {
      m_delta += amount;
    };

    void move_to(const int value) {
      int delta = value - get();
      m_delta += delta;
    };

    void apply() {
      if (m_delta == 0) return;
      CommandHistory::add(std::make_unique<ChangePrimitiveCommand<int>>(m_value, get()));
      m_delta = 0;
    };
  private:
    int m_value;
    int m_delta = 0;
  };

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

  class Vec2Value {
  public:
    Vec2Value() : m_value(0.0f) {};
    Vec2Value(const vec2 value) : m_value(value) {};
    Vec2Value(float x, float y) : m_value({ x, y }) {};
    ~Vec2Value() = default;

    inline vec2 get() const { return m_value + m_delta; };
    inline vec2 delta() const { return m_delta; };

    void set(const vec2& value) {
      if (m_value == value) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, value));
      Math::zero(m_delta);
    };

    void add(const vec2& amount) {
      if (Math::is_zero(amount)) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, m_value + amount));
    };

    void set_delta(const vec2& value) {
      m_delta = value;
    };

    void add_delta(const vec2& amount) {
      m_delta += amount;
    };

    void move_to(const vec2& value) {
      vec2 delta = value - get();
      m_delta += delta;
    };

    void apply() {
      if (Math::is_zero(m_delta)) return;
      CommandHistory::add(std::make_unique<ChangeVec2Command>(m_value, get()));
      Math::zero(m_delta);
    };
  private:
    vec2 m_value;
    vec2 m_delta{ 0.0f };
  };

}