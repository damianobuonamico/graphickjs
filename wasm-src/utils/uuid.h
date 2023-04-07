#pragma once

#include <unordered_map>
#include <stdint.h>

class UUID {
public:
  UUID();
  UUID(uint64_t uuid);
  UUID(uint32_t a, uint32_t b);
  UUID(uint32_t a, uint32_t b, uint32_t c);
  UUID(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
  UUID(const UUID& other);

  operator uint64_t () { return m_UUID; }
  operator const uint64_t() const { return m_UUID; }
private:
  uint64_t m_UUID;
};

namespace std {

  template <>
  struct hash<UUID> {
    std::size_t operator()(const UUID& uuid) const {
      return hash<uint64_t>()((uint64_t)uuid);
    }
  };

}