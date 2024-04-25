/**
 * @file encode.h
 * @brief This file contains the methods to encode data in binary format.
 */

#pragma once

#include "../../math/vec2.h"
#include "../../math/vec4.h"
#include "../../math/mat2x3.h"

#include "../../utils/assert.h"

#include <vector>
#include <string>

namespace graphick::io {

  /**
   * @brief A class to encode data in binary format.
   *
   * @struct EncodedData
   */
  struct EncodedData {
  public:
    std::vector<uint8_t> data;    /* The encoded data buffer. */

    /**
     * @brief Encodes a boolean.
     *
     * A boolean is treated as a 8-bit unsigned integer.
     *
     * @param t The boolean to encode.
     */
    inline EncodedData& boolean(const bool t) {
      data.push_back(static_cast<uint8_t>(t));
      return *this;
    }

    /**
     * @brief Encodes an int8_t.
     *
     * @param t The int8_t to encode.
     */
    inline EncodedData& int8(const int8_t t) {
      data.push_back(static_cast<uint8_t>(t));
      return *this;
    }

    /**
     * @brief Encodes an int16_t.
     *
     * @param t The int16_t to encode.
     */
    inline EncodedData& int16(const int16_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int16_t));
      return *this;
    }

    /**
     * @brief Encodes an int32_t.
     *
     * @param t The int32_t to encode.
     */
    inline EncodedData& int32(const int32_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int32_t));
      return *this;
    }

    /**
     * @brief Encodes an int64_t.
     *
     * @param t The int64_t to encode.
     */
    inline EncodedData& int64(const int64_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(int64_t));
      return *this;
    }

    /**
     * @brief Encodes a uint8_t.
     *
     * @param t The uint8_t to encode.
     */
    inline EncodedData& uint8(const uint8_t t) {
      data.push_back(t);
      return *this;
    }

    /**
     * @brief Encodes a uint16_t.
     *
     * @param t The uint16_t to encode.
     */
    inline EncodedData& uint16(const uint16_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint16_t));
      return *this;
    }

    /**
     * @brief Encodes a uint32_t.
     *
     * @param t The uint32_t to encode.
     */
    inline EncodedData& uint32(const uint32_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint32_t));
      return *this;
    }

    /**
     * @brief Encodes a uint64_t.
     *
     * @param t The uint64_t to encode.
     */
    inline EncodedData& uint64(const uint64_t t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(uint64_t));
      return *this;
    }

    /**
     * @brief Encodes a float.
     *
     * @param t The float to encode.
     */
    inline EncodedData& float32(const float t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(float));
      return *this;
    }

    /**
     * @brief Encodes a double.
     *
     * @param t The double to encode.
     */
    inline EncodedData& float64(const double t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(double));
      return *this;
    }

    /**
     * @brief Encodes a bitfield.
     *
     * A bitfield is treated as a 8-bit unsigned integer.
     *
     * @param t The bitfield to encode.
     */
    inline EncodedData& bitfield(std::initializer_list<bool> field) {
      std::initializer_list<bool>::const_iterator b = field.begin();
      uint8_t data = 0;

      for (uint8_t i = 0; i < field.size(); i++) {
        if (*b) {
          data |= (1 << i);
        }

        b++;
      }

      return uint8(data);
    }

    /**
     * @brief Encodes a component id.
     *
     * A component id is treated as a 8-bit unsigned integer.
     *
     * @param t The component id to encode.
     */
    inline EncodedData& component_id(const uint8_t t) {
      return uint8(t);
    }

    /**
     * @brief Encodes a UUID.
     *
     * An UUID is treated as a 64-bit unsigned integer.
     *
     * @param t The UUID to encode.
     */
    inline EncodedData& uuid(const uint64_t t) {
      return uint64(t);
    }

    /**
     * @brief Encodes a std::string.
     *
     * A string is encoded as a 16-bit unsigned integer representing the string's size, followed by the string's data.
     *
     * @param t The string to encode.
     */
    inline EncodedData& string(const std::string& t) {
      uint16(static_cast<uint16_t>(t.size()));
      data.insert(data.end(), t.begin(), t.end());
      return *this;
    }

    /**
     * @brief Encodes a std::vector<T>.
     *
     * A vector is encoded as a 32-bit unsigned integer representing the vector's size, followed by the vector's data.
     * The vector data is reinterpreted as a sequence of bytes.
     *
     * @param t The vector to encode.
     */
    template <typename T>
    inline EncodedData& vector(const std::vector<T>& t) {
      uint32(t.size());
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(t.data()), reinterpret_cast<const uint8_t*>(t.data()) + t.size() * sizeof(T));
      return *this;
    }

    /**
     * @brief Encodes a math::vec2.
     *
     * A vec2 is encoded as a sequence of 32-bit floating point numbers.
     *
     * @param t The vec2 to encode.
     */
    inline EncodedData& vec2(const math::vec2& t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(math::vec2));
      return *this;
    }

    /**
     * @brief Encodes a math::mat2x3.
     *
     * A mat2x3 is encoded as a sequence of 32-bit floating point numbers.
     *
     * @param t The mat2x3 to encode.
     */
    inline EncodedData& mat2x3(const math::mat2x3& t) {
      data.insert(data.end(), reinterpret_cast<const uint8_t*>(&t), reinterpret_cast<const uint8_t*>(&t) + sizeof(math::mat2x3));
      return *this;
    }

    /**
     * @brief Encodes a math::vec4 representing an RGB color.
     *
     * A vec4 is encoded as a sequence of 8-bit unsigned integers.
     *
     * @param t The vec4 to encode.
     */
    inline EncodedData& color(const vec4& t) {
      uint8(static_cast<uint8_t>(t.r * 255.0f));
      uint8(static_cast<uint8_t>(t.g * 255.0f));
      uint8(static_cast<uint8_t>(t.b * 255.0f));
      uint8(static_cast<uint8_t>(t.a * 255.0f));
      return *this;
    }
  };

  /**
   * @brief A class to decode data from binary format.
   *
   * @struct DataDecoder
   */
  struct DataDecoder {
  public:
    /**
     * @brief Constructs a new DataDecoder from an EncodedData.
     *
     * @param data The EncodedData to decode.
     */
    DataDecoder(const EncodedData* data) : m_data(data) {}

    /**
     * @brief Checks if the decoder has reached the end of the data.
     *
     * @return true if the decoder has reached the end of the data, false otherwise.
     */
    inline bool end_of_data() const {
      return m_index >= m_data->data.size();
    }

    /**
     * @brief Checks if the data has enough bytes to decode.
     *
     * @param size The number of bytes to decode.
     * @return true if the data has enough bytes to decode, false otherwise.
     */
    inline bool has_bytes(size_t size) const {
      return m_index + size <= m_data->data.size();
    }

    /**
     * @brief Decodes a boolean.
     *
     * @return The decoded boolean.
     */
    inline bool boolean() {
      GK_ASSERT(has_bytes(sizeof(uint8_t)), "Not enough bytes to decode boolean!");
      if (!has_bytes(sizeof(uint8_t))) return false;

      return m_data->data[m_index++] != 0;
    }

    /**
     * @brief Decodes an int8_t.
     *
     * @return The decoded int8_t.
     */
    inline int8_t int8() {
      GK_ASSERT(has_bytes(sizeof(int8_t)), "Not enough bytes to decode int8_t!");
      if (!has_bytes(sizeof(int8_t))) return 0;

      return static_cast<int8_t>(m_data->data[m_index++]);
    }

    /**
     * @brief Decodes an int16_t.
     *
     * @return The decoded int16_t.
     */
    inline int16_t int16() {
      GK_ASSERT(has_bytes(sizeof(int16_t)), "Not enough bytes to decode int16_t!");
      if (!has_bytes(sizeof(int16_t))) return 0;

      int16_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int16_t));
      m_index += sizeof(int16_t);

      return t;
    }

    /**
     * @brief Decodes an int32_t.
     *
     * @return The decoded int32_t.
     */
    inline int32_t int32() {
      GK_ASSERT(has_bytes(sizeof(int32_t)), "Not enough bytes to decode int32_t!");
      if (!has_bytes(sizeof(int32_t))) return 0;

      int32_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int32_t));
      m_index += sizeof(int32_t);

      return t;
    }

    /**
     * @brief Decodes an int64_t.
     *
     * @return The decoded int64_t.
     */
    inline int64_t int64() {
      GK_ASSERT(has_bytes(sizeof(int64_t)), "Not enough bytes to decode int64_t!");
      if (!has_bytes(sizeof(int64_t))) return 0;

      int64_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(int64_t));
      m_index += sizeof(int64_t);

      return t;
    }

    /**
     * @brief Decodes a uint8_t.
     *
     * @return The decoded uint8_t.
     */
    inline uint8_t uint8() {
      GK_ASSERT(has_bytes(sizeof(uint8_t)), "Not enough bytes to decode uint8_t!");
      if (!has_bytes(sizeof(uint8_t))) return 0;

      return m_data->data[m_index++];
    }

    /**
     * @brief Decodes a uint16_t.
     *
     * @return The decoded uint16_t.
     */
    inline uint16_t uint16() {
      GK_ASSERT(has_bytes(sizeof(uint16_t)), "Not enough bytes to decode uint16_t!");
      if (!has_bytes(sizeof(uint16_t))) return 0;

      uint16_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint16_t));
      m_index += sizeof(uint16_t);

      return t;
    }

    /**
     * @brief Decodes a uint32_t.
     *
     * @return The decoded uint32_t.
     */
    inline uint32_t uint32() {
      GK_ASSERT(has_bytes(sizeof(uint32_t)), "Not enough bytes to decode uint32_t!");
      if (!has_bytes(sizeof(uint32_t))) return 0;

      uint32_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint32_t));
      m_index += sizeof(uint32_t);

      return t;
    }

    /**
     * @brief Decodes a uint64_t.
     *
     * @return The decoded uint64_t.
     */
    inline uint64_t uint64() {
      GK_ASSERT(has_bytes(sizeof(uint64_t)), "Not enough bytes to decode uint64_t!");
      if (!has_bytes(sizeof(uint64_t))) return 0;

      uint64_t t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(uint64_t));
      m_index += sizeof(uint64_t);

      return t;
    }

    /**
     * @brief Decodes a float.
     *
     * @return The decoded float.
     */
    inline float float32() {
      GK_ASSERT(has_bytes(sizeof(float)), "Not enough bytes to decode float!");
      if (!has_bytes(sizeof(float))) return 0;

      float t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(float));
      m_index += sizeof(float);

      return t;
    }

    /**
     * @brief Decodes a double.
     *
     * @return The decoded double.
     */
    inline double float64() {
      GK_ASSERT(has_bytes(sizeof(double)), "Not enough bytes to decode double!");
      if (!has_bytes(sizeof(double))) return 0;

      double t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(double));
      m_index += sizeof(double);

      return t;
    }

    /**
     * @brief Decodes a bitfield.
     *
     * @tparam N The number of properties in the bitfield.
     * @return The decoded bitfield.
     */
    template <uint8_t N>
    inline std::array<bool, N> bitfield() {
      const uint8_t data = uint8();
      std::array<bool, N> field;

      for (uint8_t i = 0; i < N; i++) {
        field[i] = data & (1 << i);
      }

      return field;
    }

    /**
     * @brief Decodes a component id.
     *
     * @return The decoded component id.
     */
    inline uint8_t component_id() {
      return uint8();
    }

    /**
     * @brief Decodes a UUID.
     *
     * @return The decoded UUID.
     */
    inline uint64_t uuid() {
      return uint64();
    }

    /**
     * @brief Decodes a std::string.
     *
     * @return The decoded string.
     */
    inline std::string string() {
      uint16_t size = uint16();

      GK_ASSERT(has_bytes(size), "Not enough bytes to decode string!");
      if (!has_bytes(size)) return "";

      std::string t(m_data->data.begin() + m_index, m_data->data.begin() + m_index + size);
      m_index += size;

      return t;
    }

    /**
     * @brief Decodes a std::vector<T>.
     *
     * @return The decoded vector.
     */
    template <typename T>
    inline std::vector<T> vector() {
      uint32_t size = uint32();

      GK_ASSERT(has_bytes(size * sizeof(T)), "Not enough bytes to decode vector!");
      if (!has_bytes(size * sizeof(T))) return std::vector<T>();

      std::vector<T> t(size);
      std::memcpy(t.data(), m_data->data.data() + m_index, size * sizeof(T));
      m_index += size * sizeof(T);

      return t;
    }

    /**
     * @brief Decodes a math::vec2.
     *
     * @return The decoded vec2.
     */
    inline math::vec2 vec2() {
      GK_ASSERT(has_bytes(sizeof(math::vec2)), "Not enough bytes to decode vec2!");
      if (!has_bytes(sizeof(math::vec2))) return math::vec2();

      math::vec2 t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(math::vec2));
      m_index += sizeof(math::vec2);

      return t;
    }

    /**
     * @brief Decodes a math::mat2x3.
     *
     * @return The decoded mat2x3.
     */
    inline math::mat2x3 mat2x3() {
      GK_ASSERT(has_bytes(sizeof(math::mat2x3)), "Not enough bytes to decode mat2x3!");
      if (!has_bytes(sizeof(math::mat2x3))) return math::mat2x3();

      math::mat2x3 t;
      std::memcpy(&t, m_data->data.data() + m_index, sizeof(math::mat2x3));
      m_index += sizeof(math::mat2x3);

      return t;
    }

    /**
     * @brief Decodes a math::vec4 representing an RGB color.
     *
     * @return The decoded vec4.
     */
    inline math::vec4 color() {
      GK_ASSERT(has_bytes(4 * sizeof(uint8_t)), "Not enough bytes to decode color!");
      if (!has_bytes(4 * sizeof(uint8_t))) return math::vec4();

      math::vec4 t;
      t.r = uint8() / 255.0f;
      t.g = uint8() / 255.0f;
      t.b = uint8() / 255.0f;
      t.a = uint8() / 255.0f;

      return t;
    }
  private:
    const EncodedData* m_data;    /* A pointer to the underlying data to decode. */
    size_t m_index = 0;           /* The current index in the data. */
  };

}
