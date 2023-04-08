#pragma once

#include "../math/vec2.h"
#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/box.h"
#include "uuid.h"

#include <deque>
#include <map>
#include <string>
#include <initializer_list>

class JSON {
public:
  enum class Class {
    Null,
    Object,
    Array,
    String,
    Float,
    Int,
    Bool
  };

  template <typename Container>
  class JSONWrapper {
  public:
    JSONWrapper(Container* val): m_object(val) {}
    JSONWrapper(std::nullptr_t): m_object(nullptr) {}

    typename Container::iterator begin() { return m_object ? m_object->begin() : typename Container::iterator(); }
    typename Container::iterator end() { return m_object ? m_object->end() : typename Container::iterator(); }
    typename Container::const_iterator begin() const { return m_object ? m_object->begin() : typename Container::iterator(); }
    typename Container::const_iterator end() const { return m_object ? m_object->end() : typename Container::iterator(); }
  private:
    Container* m_object;
  };

  template <typename Container>
  class JSONConstWrapper {
  public:
    JSONConstWrapper(const Container* val): m_object(val) {}
    JSONConstWrapper(std::nullptr_t): m_object(nullptr) {}

    typename Container::const_iterator begin() const { return m_object ? m_object->begin() : typename Container::const_iterator(); }
    typename Container::const_iterator end() const { return m_object ? m_object->end() : typename Container::const_iterator(); }
  private:
    const Container* m_object;
  };
public:
  JSON(): m_internal(), m_type(Class::Null) {}
  JSON(std::initializer_list<JSON> list);
  JSON(JSON&& other);
  JSON(const JSON& other);

  JSON& operator=(JSON&& other);
  JSON& operator=(const JSON& other);
  JSON& operator=(UUID other);
  JSON& operator=(vec2 other);
  JSON& operator=(const vec3& other);
  JSON& operator=(const vec4& other);
  JSON& operator=(const Box& other);

  ~JSON();

  template <typename T>
  JSON(T b, typename std::enable_if<std::is_same<T, bool>::value>::type* = 0): m_internal(b), m_type(Class::Bool) {}
  template <typename T>
  JSON(T i, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type* = 0) : m_internal((int)i), m_type(Class::Int) {}
  template <typename T>
  JSON(T f, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0) : m_internal((float)f), m_type(Class::Float) {}
  template <typename T>
  JSON(T s, typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = 0) : m_internal(std::string(s)), m_type(Class::String) {}

  JSON(std::nullptr_t): m_internal(), m_type(Class::Null) {}

  static JSON make(Class type);
  static JSON load(const std::string& str);

  static JSON array() { return std::move(JSON::make(JSON::Class::Array)); }
  template <typename... T>
  static JSON array(T... args) {
    JSON arr = make(JSON::Class::Array);
    arr.append(args...);
    return std::move(arr);
  }
  static JSON object() { return std::move(make(JSON::Class::Object)); }

  template <typename T>
  void append(T arg) { set_type(Class::Array); m_internal.m_list->emplace_back(arg); }
  template <typename T, typename... U>
  void append(T arg, U... args) { append(arg); append(args...); }

  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value, JSON&>::type operator=(T b) {
    set_type(Class::Bool); m_internal.m_bool = b; return *this;
  }
  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, JSON&>::type operator=(T i) {
    set_type(Class::Int); m_internal.m_int = i; return *this;
  }
  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, JSON&>::type operator=(T f) {
    set_type(Class::Float); m_internal.m_float = f; return *this;
  }
  template <typename T>
  typename std::enable_if<std::is_convertible<T, std::string>::value, JSON&>::type operator=(T s) {
    set_type(Class::String); *m_internal.m_string = std::string(s); return *this;
  }

  JSON& operator[](const std::string& key);
  JSON& operator[](unsigned int index);

  JSON& at(const std::string& key);
  const JSON& at(const std::string& key) const;
  JSON& at(unsigned int index);
  const JSON& at(unsigned int index) const;

  int length() const;
  bool has(const std::string& key) const;
  int size() const;

  Class type() const { return m_type; }
  bool is_null() const { return m_type == Class::Null; }

  std::string to_string() const;
  std::string to_string(bool& ok) const;

  float to_float() const;
  float to_float(bool& ok) const;

  int to_int() const;
  int to_int(bool& ok) const;

  bool to_bool() const;
  bool to_bool(bool& ok) const;

  vec2 to_vec2() const;
  vec2 to_vec2(bool& ok) const;

  vec3 to_vec3() const;
  vec3 to_vec3(bool& ok) const;

  vec4 to_vec4() const;
  vec4 to_vec4(bool& ok) const;

  JSONWrapper<std::map<std::string, JSON>> object_range();
  JSONWrapper<std::deque<JSON>> array_range();
  JSONConstWrapper<std::map<std::string, JSON>> object_range() const;
  JSONConstWrapper<std::deque<JSON>> array_range() const;

  std::string dump() const;

  friend std::ostream& operator<<(std::ostream& os, const JSON& json);
private:
  void set_type(Class type);
  void clear_internal();
private:
  union Data {
    Data(float d): m_float(d) {}
    Data(int l): m_int(l) {}
    Data(bool b): m_bool(b) {}
    Data(std::string s): m_string(new std::string(s)) {}
    Data(): m_int(0) {}

    std::deque<JSON>* m_list;
    std::map<std::string, JSON>* m_map;
    std::string* m_string;
    float m_float;
    int m_int;
    bool m_bool;
  } m_internal;

  Class m_type = Class::Null;
};
