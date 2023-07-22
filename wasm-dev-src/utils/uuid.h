#pragma once

#include <unordered_map>
#include <cstdint>

namespace Graphick::Utils {

  class uuid {
  public:
    uuid();
    uuid(uint64_t uuid);
    uuid(const uuid& other);

    uuid& operator=(const uint64_t other);

    operator uint64_t () { return m_uuid; }
    operator const uint64_t() const { return m_uuid; }
  private:
    uint64_t m_uuid;
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
