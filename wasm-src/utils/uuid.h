/**
 * @file uuid.h
 * @brief The file contains the definition of the UUID class.
 */

#pragma once

#include <unordered_map>
#include <cstdint>

namespace Graphick::Utils {

  /**
   * @brief The class that represents an universally unique identifier.
   *
   * The class is used to identify resources and objects in the system.
   * It is a 64-bit unsigned integer under the hood.
   *
   * @class uuid
   */
  class uuid {
  public:
    static const uuid null;    /* The null UUID, corresponding to 0. */
  public:
    /**
     * @brief Default constructors.
     */
    uuid();
    uuid(uint64_t uuid);
    uuid(const uuid& other);

    /**
     * @brief Default copy assignment operator.
     */
    uuid& operator=(const uint64_t other);

    /**
     * @brief Type casts to uint64_t operators.
     */
    operator uint64_t () { return m_uuid; }
    operator const uint64_t() const { return m_uuid; }
  private:
    uint64_t m_uuid;    /* The 64-bit unsigned integer that represents the UUID. */
  };

}

namespace Graphick {
  using uuid = Utils::uuid;
}

namespace std {

  template <>
  struct hash<Graphick::Utils::uuid> {
    std::size_t operator()(const Graphick::Utils::uuid& uuid) const {
      return hash<uint64_t>()((uint64_t)uuid);
    }
  };

}
