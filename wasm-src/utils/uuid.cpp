#include "uuid.h"

#include <random>

static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

UUID::UUID() : m_UUID(s_uniform_distribution(s_engine)) {}

UUID::UUID(uint64_t uuid) : m_UUID(uuid) {}

UUID::UUID(const UUID& other) : m_UUID(other.m_UUID) {}
