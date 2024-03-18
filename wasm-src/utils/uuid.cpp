/**
 * @file uuid.cpp
 * @brief The file contains the implementation of the UUID class.
 */

#include "uuid.h"

#include <random>

namespace Graphick::Utils {

  static std::random_device s_random_device;
  static std::mt19937_64 s_engine(s_random_device());
  static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

  const uuid uuid::null = 0;

  uuid::uuid() : m_uuid(s_uniform_distribution(s_engine)) {}

  uuid::uuid(uint64_t uuid) : m_uuid(uuid) {}

  uuid::uuid(const uuid& other) : m_uuid(other.m_uuid) {}

  uuid& uuid::operator=(const uint64_t other) {
    m_uuid = other;
    return *this;
  }

}
