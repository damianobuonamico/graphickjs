#pragma once

#include "../math/vec2.h"
#include "../math/vector.h"

#include <vector>

namespace Graphick::History {

  class BoolValue {
  public:
    BoolValue() : m_value(false) {};
    BoolValue(const bool value) : m_value(value) {};

    inline bool get() const { return m_value; };

    void set(const bool value);
  private:
    bool m_value;
  };

  class FloatValue {
  public:
    FloatValue() : m_value(0.0f) {};
    FloatValue(const float value) : m_value(value) {};

    inline float get() const { return m_value + m_delta; };
    inline float delta() const { return m_delta; };

    void set(const float value);
    void add(const float amount);

    inline void set_delta(const float value) { m_delta = value; }
    inline void add_delta(const float amount) { m_delta += amount; }
    inline void move_to(const float value) { m_delta += value - get(); }

    void apply();
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

    void set(const int value);
    void add(const int amount);

    inline void set_delta(const int value) { m_delta = value; }
    inline void add_delta(const int amount) { m_delta += amount; }
    inline void move_to(const int value) { m_delta += value - get(); }

    void apply();
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

    void insert(const value& pair);
    void insert(const value& pair, int index);
    void erase(const value& pair);
    void erase(const value& pair, int index);
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

    void set(const vec2& value);
    void add(const vec2& amount);

    inline void set_delta(const vec2& value) { m_delta = value; }
    inline void add_delta(const vec2& amount) { m_delta += amount; }
    inline void move_to(const vec2& value) { m_delta += value - get(); }

    void apply();
  private:
    vec2 m_value;
    vec2 m_delta = { 0.0f, 0.0f };
  };

}