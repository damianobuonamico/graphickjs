/**
 * @file encode.h
 * @brief This file contains the declaration of the methods to encode data in binary format.
 *
 * @todo doc
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"
#include "../../math/mat2x3.h"

#include "../../utils/assert.h"

#include <vector>
#include <string>

namespace Graphick::Renderer::Geometry {
  class Path;
}

namespace Graphick::io {

  struct EncodedData {
  public:
    std::vector<uint8_t> data;

    inline EncodedData& int8(const int8_t t) {
      data.push_back(static_cast<uint8_t>(t));
      return *this;
    }

    inline EncodedData& int16(const int16_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int16_t));
      return *this;
    }

    inline EncodedData& int32(const int32_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int32_t));
      return *this;
    }

    inline EncodedData& int64(const int64_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int64_t));
      return *this;
    }

    inline EncodedData& uint8(const uint8_t t) {
      data.push_back(t);
      return *this;
    }

    inline EncodedData& uint16(const uint16_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint16_t));
      return *this;
    }

    inline EncodedData& uint32(const uint32_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint32_t));
      return *this;
    }

    inline EncodedData& uint64(const uint64_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint64_t));
      return *this;
    }

    inline EncodedData& float32(const float t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(float));
      return *this;
    }

    inline EncodedData& float64(const double t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(double));
      return *this;
    }

    inline EncodedData& component_id(const uint8_t t) {
      return uint8(t);
    }

    inline EncodedData& uuid(const uint64_t t) {
      return uint64(t);
    }

    inline EncodedData& string(const std::string& t) {
      uint16(static_cast<uint16_t>(t.size()));
      data.insert(data.end(), t.begin(), t.end());
      return *this;
    }

    template<typename T>
    inline EncodedData& vector(const std::vector<T>& t) {
      uint32(t.size());
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(t.data()), reinterpret_cast<const uint8_t*>(t.data()) + t.size() * sizeof(T));
      return *this;
    }

    inline EncodedData& vec2(const Math::vec2& t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(Math::vec2));
      return *this;
    }

    inline EncodedData& mat2x3(const Math::mat2x3& t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(Math::mat2x3));
      return *this;
    }

    inline EncodedData& color(const vec4& t) {
      uint8(static_cast<uint8_t>(t.r * 255.0f));
      uint8(static_cast<uint8_t>(t.g * 255.0f));
      uint8(static_cast<uint8_t>(t.b * 255.0f));
      uint8(static_cast<uint8_t>(t.a * 255.0f));
      return *this;
    }
  };

  struct DataDecoder {
  public:
    DataDecoder(const EncodedData* data) : m_data(data) {}

    inline bool end_of_data() const {
      return m_index >= m_data->data.size();
    }

    inline bool has_bytes(size_t size) const {
      return m_index + size <= m_data->data.size();
    }

    inline int8_t int8() {
      GK_ASSERT(has_bytes(sizeof(int8_t)), "Not enough bytes to decode int8_t!");
      if (!has_bytes(sizeof(int8_t))) return 0;

      return static_cast<int8_t>(m_data->data[m_index++]);
    }

    inline int16_t int16() {
      GK_ASSERT(has_bytes(sizeof(int16_t)), "Not enough bytes to decode int16_t!");
      if (!has_bytes(sizeof(int16_t))) return 0;

      int16_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int16_t));
      m_index += sizeof(int16_t);

      return t;
    }

    inline int32_t int32() {
      GK_ASSERT(has_bytes(sizeof(int32_t)), "Not enough bytes to decode int32_t!");
      if (!has_bytes(sizeof(int32_t))) return 0;

      int32_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int32_t));
      m_index += sizeof(int32_t);

      return t;
    }

    inline int64_t int64() {
      GK_ASSERT(has_bytes(sizeof(int64_t)), "Not enough bytes to decode int64_t!");
      if (!has_bytes(sizeof(int64_t))) return 0;

      int64_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int64_t));
      m_index += sizeof(int64_t);

      return t;
    }

    inline uint8_t uint8() {
      GK_ASSERT(has_bytes(sizeof(uint8_t)), "Not enough bytes to decode uint8_t!");
      if (!has_bytes(sizeof(uint8_t))) return 0;

      return m_data->data[m_index++];
    }

    inline uint16_t uint16() {
      GK_ASSERT(has_bytes(sizeof(uint16_t)), "Not enough bytes to decode uint16_t!");
      if (!has_bytes(sizeof(uint16_t))) return 0;

      uint16_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint16_t));
      m_index += sizeof(uint16_t);

      return t;
    }

    inline uint32_t uint32() {
      GK_ASSERT(has_bytes(sizeof(uint32_t)), "Not enough bytes to decode uint32_t!");
      if (!has_bytes(sizeof(uint32_t))) return 0;

      uint32_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint32_t));
      m_index += sizeof(uint32_t);

      return t;
    }

    inline uint64_t uint64() {
      GK_ASSERT(has_bytes(sizeof(uint64_t)), "Not enough bytes to decode uint64_t!");
      if (!has_bytes(sizeof(uint64_t))) return 0;

      uint64_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint64_t));
      m_index += sizeof(uint64_t);

      return t;
    }

    inline float float32() {
      GK_ASSERT(has_bytes(sizeof(float)), "Not enough bytes to decode float!");
      if (!has_bytes(sizeof(float))) return 0;

      float t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(float));
      m_index += sizeof(float);

      return t;
    }

    inline double float64() {
      GK_ASSERT(has_bytes(sizeof(double)), "Not enough bytes to decode double!");
      if (!has_bytes(sizeof(double))) return 0;

      double t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(double));
      m_index += sizeof(double);

      return t;
    }

    inline uint8_t component_id() {
      return uint8();
    }

    inline uint64_t uuid() {
      return uint64();
    }

    inline std::string string() {
      uint16_t size = uint16();

      GK_ASSERT(has_bytes(size), "Not enough bytes to decode string!");
      if (!has_bytes(size)) return "";

      std::string t(m_data->data.begin() + m_index, m_data->data.begin() + m_index + size);
      m_index += size;

      return t;
    }

    template<typename T>
    inline std::vector<T> vector() {
      uint32_t size = uint32();

      GK_ASSERT(has_bytes(size * sizeof(T)), "Not enough bytes to decode vector!");
      if (!has_bytes(size * sizeof(T))) return std::vector<T>();

      std::vector<T> t(size);
      std::memcpy(t.data(), m_data->data.data() + m_index, size * sizeof(T));
      m_index += size * sizeof(T);

      return t;
    }

    inline Math::vec2 vec2() {
      GK_ASSERT(has_bytes(sizeof(Math::vec2)), "Not enough bytes to decode vec2!");
      if (!has_bytes(sizeof(Math::vec2))) return Math::vec2();

      Math::vec2 t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(Math::vec2));
      m_index += sizeof(Math::vec2);

      return t;
    }

    inline Math::mat2x3 mat2x3() {
      GK_ASSERT(has_bytes(sizeof(Math::mat2x3)), "Not enough bytes to decode mat2x3!");
      if (!has_bytes(sizeof(Math::mat2x3))) return Math::mat2x3();

      Math::mat2x3 t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(Math::mat2x3));
      m_index += sizeof(Math::mat2x3);

      return t;
    }

    inline Math::vec4 color() {
      GK_ASSERT(has_bytes(4 * sizeof(uint8_t)), "Not enough bytes to decode color!");
      if (!has_bytes(4 * sizeof(uint8_t))) return Math::vec4();

      Math::vec4 t;
      t.r = uint8() / 255.0f;
      t.g = uint8() / 255.0f;
      t.b = uint8() / 255.0f;
      t.a = uint8() / 255.0f;

      return t;
    }
  private:
    const EncodedData* m_data;
    size_t m_index = 0;
  };

}
