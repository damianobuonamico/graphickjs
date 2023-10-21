#pragma once

#include "../math/vec2.h"

#include "../utils/uuid.h"

#include <vector>

namespace Graphick::History {

  class BoolValue {
  public:
    BoolValue() : m_value(false) {};
    BoolValue(const bool value) : m_value(value) {};

    inline bool get() const { return m_value; };

    void set(const bool value);

    inline void operator=(const bool value) { set(value); }

    inline operator bool() const { return m_value; }
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

  // TODO: value operators
  class UUIDValue {
  public:
    UUIDValue() = default;
    UUIDValue(const uuid value) : m_value(value) {}

    inline uuid get() const { return m_value; }

    void set(const uuid value);

    inline void operator=(const uuid value) { set(value); }

    inline operator uuid() const { return m_value; }
    inline operator bool() const { return m_value != 0; }
  private:
    uuid m_value;
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

  template <typename T>
  class VectorValue {
  public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
  public:
    VectorValue(std::initializer_list<T> list) : m_value(list) {}
    VectorValue(const std::vector<T>& vector) : m_value(vector) {}
    VectorValue(const VectorValue& vector) : m_value(vector.m_value) {}
    VectorValue() = default;

    ~VectorValue() = default;

    inline iterator begin() { return m_value.begin(); }
    inline iterator end() { return m_value.end(); }
    inline const_iterator begin() const { return m_value.begin(); }
    inline const_iterator end() const { return m_value.end(); }

    inline std::reverse_iterator<iterator> rbegin() { return m_value.rbegin(); }
    inline std::reverse_iterator<iterator> rend() { return m_value.rend(); }
    inline std::reverse_iterator<const_iterator> rbegin() const { return m_value.rbegin(); }
    inline std::reverse_iterator<const_iterator> rend() const { return m_value.rend(); }

    inline size_t size() const { return m_value.size(); }
    inline bool empty() const { return m_value.empty(); }

    inline T& operator[](size_t index) { return m_value[index]; }
    inline T& front() { return m_value.front(); }
    inline T& back() { return m_value.back(); }
    inline const T& front() const { return m_value.front(); }
    inline const T& back() const { return m_value.back(); }

    void push_back(const T& value);
    void insert(const T& value, int index);
    void pop_back();
    void erase(const T& value);
    void erase(int index);
    void clear();
  private:
    std::vector<T> m_value;
  };

}
