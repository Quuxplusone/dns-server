#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

class RRClass {
public:
    enum detail : uint16_t {
        IN = 1,
        ANY = 255,
    };

    explicit constexpr RRClass() noexcept = default;
    explicit constexpr RRClass(int x) noexcept : m_value(x) {}
    explicit RRClass(const std::string& repr);
    constexpr RRClass(enum RRClass::detail x) noexcept : m_value(int(x)) {}
    explicit constexpr operator int() const noexcept { return m_value; }
    constexpr bool operator==(RRClass rhs) const noexcept { return m_value == rhs.m_value; }
    constexpr bool operator!=(RRClass rhs) const noexcept { return m_value != rhs.m_value; }

    std::string repr() const {
        switch (m_value) {
            case IN: return "IN";
            case ANY: return "ANY";
            default: return "CLASS" + std::to_string(int(m_value));
        }
    }

private:
    uint16_t m_value = 0;
};

class RRType {
public:
    enum detail : uint16_t {
        A = 1,
        NS = 2,
        CNAME = 5,
        SOA = 6,
        PTR = 12,
        MX = 15,
        TXT = 16,
        ANY = 255,
    };

    explicit constexpr RRType() noexcept = default;
    explicit constexpr RRType(int x) noexcept : m_value(x) {}
    explicit RRType(const std::string& repr);
    constexpr RRType(enum RRType::detail x) noexcept : m_value(int(x)) {}
    explicit constexpr operator int() const noexcept { return m_value; }
    constexpr bool operator==(RRType rhs) const noexcept { return m_value == rhs.m_value; }
    constexpr bool operator!=(RRType rhs) const noexcept { return m_value != rhs.m_value; }

    std::string repr() const {
        switch (m_value) {
            case A: return "A";
            case NS: return "NS";
            case CNAME: return "CNAME";
            case SOA: return "SOA";
            case PTR: return "PTR";
            case MX: return "MX";
            case TXT: return "TXT";
            case ANY: return "ANY";
            default: return "TYPE" + std::to_string(int(m_value));
        }
    }

private:
    uint16_t m_value = 0;
};

} // namespace dns
