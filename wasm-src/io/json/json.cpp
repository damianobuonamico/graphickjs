#include "json.h"

#include "../../utils/console.h"

JSON parse_next(const std::string&, size_t&);

void consume_ws(const std::string& str, size_t& offset) {
  while (isspace(str[offset])) ++offset;
}

JSON parse_object(const std::string& str, size_t& offset) {
  JSON object = JSON::make(JSON::Class::Object);

  ++offset;
  consume_ws(str, offset);
  if (str[offset] == '}') {
    ++offset; return std::move(object);
  }

  while (true) {
    JSON key = parse_next(str, offset);

    consume_ws(str, offset);
    if (str[offset] != ':') {
      console::error("Object: Expected colon, found", str[offset]);
      break;
    }
    consume_ws(str, ++offset);

    JSON value = parse_next(str, offset);
    object[key.to_string()] = value;

    consume_ws(str, offset);
    if (str[offset] == ',') {
      ++offset; continue;
    } else if (str[offset] == '}') {
      ++offset; break;
    } else {
      console::error("Object: Expected comma, found", str[offset]);
      break;
    }
  }

  return std::move(object);
}

JSON parse_array(const std::string& str, size_t& offset) {
  JSON array = JSON::make(JSON::Class::Array);
  unsigned int index = 0;

  ++offset;
  consume_ws(str, offset);
  if (str[offset] == ']') {
    ++offset; return std::move(array);
  }

  while (true) {
    array[index++] = parse_next(str, offset);
    consume_ws(str, offset);

    if (str[offset] == ',') {
      ++offset; continue;
    } else if (str[offset] == ']') {
      ++offset; break;
    } else {
      console::error("Object: Array: Expected ',' or ']', found", str[offset]);
      return std::move(JSON::make(JSON::Class::Array));
    }
  }

  return std::move(array);
}

JSON parse_string(const std::string& str, size_t& offset) {
  JSON string;
  std::string val;

  for (char c = str[++offset]; c != '\"'; c = str[++offset]) {
    if (c == '\\') {
      switch (str[++offset]) {
      case '\"': val += '\"'; break;
      case '\\': val += '\\'; break;
      case '/': val += '/'; break;
      case 'b': val += '\b'; break;
      case 'f': val += '\f'; break;
      case 'n': val += '\n'; break;
      case 'r': val += '\r'; break;
      case 't': val += '\t'; break;
      case 'u': {
        val += "\\u";
        for (unsigned int i = 1; i <= 4; ++i) {
          c = str[offset + i];
          if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            val += c;
          } else {
            console::error("String: Expected hex character in unicode escape, found", c);
            return std::move(JSON::make(JSON::Class::String));
          }
        }
        offset += 4;
      } break;
      default: val += '\\'; break;
      }
    } else {
      val += c;
    }
  }

  ++offset;
  string = val;

  return std::move(string);
}

JSON parse_number(const std::string& str, size_t& offset) {
  JSON number;
  std::string val, exp_str;
  char c;
  bool is_float = false;
  long exp = 0;

  while (true) {
    c = str[offset++];
    if ((c == '-') || (c >= '0' && c <= '9')) {
      val += c;
    } else if (c == '.') {
      val += c;
      is_float = true;
    } else break;
  }

  if (c == 'E' || c == 'e') {
    c = str[offset++];
    if (c == '-') { ++offset; exp_str += '-'; }
    while (true) {
      c = str[offset++];
      if (c >= '0' && c <= '9') {
        exp_str += c;
      } else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
        console::error("Number: Expected a number for exponent, found", c);
        return std::move(JSON::make(JSON::Class::Null));
      } else break;
    }
    exp = std::stol(exp_str);
  } else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
    console::error("Number: unexpected character", c);
    return std::move(JSON::make(JSON::Class::Null));
  }

  --offset;

  if (is_float) {
    number = std::stod(val) * std::pow(10, exp);
  } else {
    if (!exp_str.empty()) {
      number = std::stol(val) * std::pow(10, exp);
    } else {
      number = std::stol(val);
    }
  }

  return std::move(number);
}

JSON parse_bool(const std::string& str, size_t& offset) {
  JSON boolean;

  if (str.substr(offset, 4) == "true") {
    boolean = true;
  } else if (str.substr(offset, 5) == "false") {
    boolean = false;
  } else {
    console::error("Bool: Expected 'true' or 'false', found", str.substr(offset, 5));
    return std::move(JSON::make(JSON::Class::Null));
  }

  offset += (boolean.to_bool() ? 4 : 5);
  return std::move(boolean);
}

JSON parse_null(const std::string& str, size_t& offset) {
  JSON null;

  if (str.substr(offset, 4) != "null") {
    console::error("ERROR: Null: Expected 'null', found", str.substr(offset, 4));
    return std::move(JSON::make(JSON::Class::Null));
  }

  offset += 4;
  return std::move(null);
}

JSON parse_next(const std::string& str, size_t& offset) {
  char value;
  consume_ws(str, offset);
  value = str[offset];

  switch (value) {
  case '[': return std::move(parse_array(str, offset));
  case '{': return std::move(parse_object(str, offset));
  case '\"': return std::move(parse_string(str, offset));
  case 't':
  case 'f': return std::move(parse_bool(str, offset));
  case 'n': return std::move(parse_null(str, offset));
  default: if ((value <= '9' && value >= '0') || value == '-')
    return std::move(parse_number(str, offset));
  }

  console::error("ERROR: Parse: Unknown starting character", value);
  return JSON();
}

static std::string json_escape(const std::string& str) {
  std::string output;

  for (unsigned int i = 0; i < str.length(); ++i) {
    switch (str[i]) {
    case '\"': output += "\\\""; break;
    case '\\': output += "\\\\"; break;
    case '\b': output += "\\b";  break;
    case '\f': output += "\\f";  break;
    case '\n': output += "\\n";  break;
    case '\r': output += "\\r";  break;
    case '\t': output += "\\t";  break;
    default: output += str[i]; break;
    }
  }

  return std::move(output);
}

JSON::JSON(std::initializer_list<JSON> list): JSON() {
  set_type(Class::Object);

  for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i) {
    operator[](i->to_string()) = *std::next(i);
  }
}

JSON::JSON(JSON&& other): m_internal(other.m_internal), m_type(other.m_type) {
  other.m_type = Class::Null; other.m_internal.m_map = nullptr;
}

JSON::JSON(const JSON& other) {
  switch (other.m_type) {
  case Class::Object:
    m_internal.m_map =
      new std::map<std::string, JSON>(other.m_internal.m_map->begin(),
        other.m_internal.m_map->end());
    break;
  case Class::Array:
    m_internal.m_list =
      new std::deque<JSON>(other.m_internal.m_list->begin(),
        other.m_internal.m_list->end());
    break;
  case Class::String:
    m_internal.m_string =
      new std::string(*other.m_internal.m_string);
    break;
  default:
    m_internal = other.m_internal;
  }
  m_type = other.m_type;
}

JSON& JSON::operator=(JSON&& other) {
  clear_internal();

  m_internal = other.m_internal;
  m_type = other.m_type;
  other.m_internal.m_map = nullptr;
  other.m_type = Class::Null;

  return *this;
}

JSON& JSON::operator=(const JSON& other) {
  clear_internal();
  switch (other.m_type) {
  case Class::Object:
    m_internal.m_map =
      new std::map<std::string, JSON>(other.m_internal.m_map->begin(),
        other.m_internal.m_map->end());
    break;
  case Class::Array:
    m_internal.m_list =
      new std::deque<JSON>(other.m_internal.m_list->begin(),
        other.m_internal.m_list->end());
    break;
  case Class::String:
    m_internal.m_string =
      new std::string(*other.m_internal.m_string);
    break;
  default:
    m_internal = other.m_internal;
  }
  m_type = other.m_type;
  return *this;
}

JSON& JSON::operator=(UUID other) {
  return operator=(std::to_string(other));
}

JSON& JSON::operator=(vec2 other) {
  JSON vec = JSON::array(other.x, other.y);
  return operator=(vec);
}

JSON& JSON::operator=(const vec3& other) {
  JSON vec = JSON::array(other.x, other.y, other.z);
  return operator=(vec);
}

JSON& JSON::operator=(const vec4& other) {
  JSON vec = JSON::array(other.x, other.y, other.z, other.w);
  return operator=(vec);
}

JSON& JSON::operator=(const Box& other) {
  JSON box = JSON::array(JSON::array(other.min.x, other.min.y), JSON::array(other.max.x, other.max.y));
  return operator=(box);
}

JSON::~JSON() {
  switch (m_type) {
  case Class::Array:
    delete m_internal.m_list;
    break;
  case Class::Object:
    delete m_internal.m_map;
    break;
  case Class::String:
    delete m_internal.m_string;
    break;
  default:;
  }
}

JSON JSON::make(Class type) {
  JSON ret; ret.set_type(type);
  return ret;
}

JSON JSON::load(const std::string& str) {
  size_t offset = 0;
  return std::move(parse_next(str, offset));
}

JSON& JSON::operator[](const std::string& key) {
  set_type(Class::Object);
  return m_internal.m_map->operator[](key);
}

JSON& JSON::operator[](unsigned int index) {
  set_type(Class::Array);

  if (index >= m_internal.m_list->size()) {
    m_internal.m_list->resize(index + 1);
  }

  return m_internal.m_list->operator[](index);
}

JSON& JSON::at(const std::string& key) {
  return operator[](key);
}

const JSON& JSON::at(const std::string& key) const {
  return m_internal.m_map->at(key);
}

JSON& JSON::at(unsigned int index) {
  return operator[](index);
}

const JSON& JSON::at(unsigned int index) const {
  return m_internal.m_list->at(index);
}

int JSON::length() const {
  if (m_type == Class::Array)
    return m_internal.m_list->size();
  else
    return -1;
}

bool JSON::has(const std::string& key) const {
  if (m_type == Class::Object) {
    return m_internal.m_map->find(key) != m_internal.m_map->end();
  }

  return false;
}

int JSON::size() const {
  if (m_type == Class::Object) {
    return m_internal.m_map->size();
  } else if (m_type == Class::Array) {
    return m_internal.m_list->size();
  } else {
    return -1;
  }
}

std::string JSON::to_string() const {
  bool b;
  return std::move(to_string(b));
}

std::string JSON::to_string(bool& ok) const {
  ok = (m_type == Class::String);
  return ok ? std::move(json_escape(*m_internal.m_string)) : std::string("");
}

float JSON::to_float() const {
  bool b;
  return to_float(b);
}

float JSON::to_float(bool& ok) const {
  bool is_float = (m_type == Class::Float);
  ok = (is_float || m_type == Class::Int);

  if (ok) {
    if (is_float) {
      return m_internal.m_float;
    }
    return static_cast<float>(m_internal.m_int);
  }

  return 0.0f;
}

int JSON::to_int() const {
  bool b;
  return to_int(b);
}

int JSON::to_int(bool& ok) const {
  bool is_int = (m_type == Class::Int);
  ok = (is_int || m_type == Class::Float);

  if (ok) {
    if (is_int) {
      return m_internal.m_int;
    }
    return static_cast<int>(m_internal.m_float);
  }

  return 0;
}

bool JSON::to_bool() const {
  bool b;
  return to_bool(b);
}

bool JSON::to_bool(bool& ok) const {
  ok = (m_type == Class::Bool);
  return ok ? m_internal.m_bool : false;
}

vec2 JSON::to_vec2() const {
  bool b;
  return to_vec2(b);
}

vec2 JSON::to_vec2(bool& ok) const {
  ok = (m_type == Class::Array);

  if (ok) {
    ok = (m_internal.m_list->size() >= 2);

    if (ok) {
      return vec2{
        m_internal.m_list->at(0).to_float(),
        m_internal.m_list->at(1).to_float()
      };
    }
  }

  return vec2{ 0.0f };
}

vec3 JSON::to_vec3() const {
  bool b;
  return to_vec3(b);
}

vec3 JSON::to_vec3(bool& ok) const {
  ok = (m_type == Class::Array);

  if (ok) {
    ok = (m_internal.m_list->size() >= 3);

    if (ok) {
      return vec3{
        m_internal.m_list->at(0).to_float(),
        m_internal.m_list->at(1).to_float(),
        m_internal.m_list->at(2).to_float()
      };
    }
  }

  return vec3{ 0.0f };
}

vec4 JSON::to_vec4() const {
  bool b;
  return to_vec4(b);
}

vec4 JSON::to_vec4(bool& ok) const {
  ok = (m_type == Class::Array);

  if (ok) {
    ok = (m_internal.m_list->size() >= 4);

    if (ok) {
      return vec4{
        m_internal.m_list->at(0).to_float(),
        m_internal.m_list->at(1).to_float(),
        m_internal.m_list->at(2).to_float(),
        m_internal.m_list->at(3).to_float()
      };
    }
  }

  return vec4{ 0.0f };
}

JSON::JSONWrapper<std::map<std::string, JSON>> JSON::object_range() {
  if (m_type == Class::Object) {
    return JSONWrapper<std::map<std::string, JSON>>(m_internal.m_map);
  }

  return JSONWrapper<std::map<std::string, JSON>>(nullptr);
}

JSON::JSONWrapper<std::deque<JSON>> JSON::array_range() {
  if (m_type == Class::Array) {
    return JSONWrapper<std::deque<JSON>>(m_internal.m_list);
  }

  return JSONWrapper<std::deque<JSON>>(nullptr);
}

JSON::JSONConstWrapper<std::map<std::string, JSON>> JSON::object_range() const {
  if (m_type == Class::Object) {
    return JSONConstWrapper<std::map<std::string, JSON>>(m_internal.m_map);
  }

  return JSONConstWrapper<std::map<std::string, JSON>>(nullptr);
}

JSON::JSONConstWrapper<std::deque<JSON>> JSON::array_range() const {
  if (m_type == Class::Array) {
    return JSONConstWrapper<std::deque<JSON>>(m_internal.m_list);
  }

  return JSONConstWrapper<std::deque<JSON>>(nullptr);
}

std::string JSON::dump() const {
  switch (m_type) {
  case Class::Null:
    return "null";
  case Class::Object: {
    std::string s = "{";
    bool skip = true;

    for (auto& p : *m_internal.m_map) {
      if (!skip) s += ",";
      s += ("\"" + p.first + "\":" + p.second.dump());
      skip = false;
    }

    s += "}";
    return s;
  }
  case Class::Array: {
    std::string s = "[";
    bool skip = true;

    for (auto& p : *m_internal.m_list) {
      if (!skip) s += ",";
      s += p.dump();
      skip = false;
    }

    s += "]";
    return s;
  }
  case Class::String:
    return "\"" + json_escape(*m_internal.m_string) + "\"";
  case Class::Float:
    return std::to_string(m_internal.m_float);
  case Class::Int:
    return std::to_string(m_internal.m_int);
  case Class::Bool:
    return m_internal.m_bool ? "true" : "false";
  default:
    return "";
  }

  return "";
}

std::ostream& operator<<(std::ostream& os, const JSON& json) {
  os << json.dump();
  return os;
}

void JSON::set_type(Class type) {
  if (m_type == type) return;

  clear_internal();

  switch (type) {
  case Class::Null:
    m_internal.m_map = nullptr;
    break;
  case Class::Object:
    m_internal.m_map = new std::map<std::string, JSON>();
    break;
  case Class::Array:
    m_internal.m_list = new std::deque<JSON>();
    break;
  case Class::String:
    m_internal.m_string = new std::string();
    break;
  case Class::Float:
    m_internal.m_float = 0.0;
    break;
  case Class::Int:
    m_internal.m_int = 0;
    break;
  case Class::Bool:
    m_internal.m_bool = false;
    break;
  }

  m_type = type;
}

void JSON::clear_internal() {
  switch (m_type) {
  case Class::Object:
    delete m_internal.m_map;
    break;
  case Class::Array:
    delete m_internal.m_list;
    break;
  case Class::String:
    delete m_internal.m_string;
    break;
  default:;
  }
}
