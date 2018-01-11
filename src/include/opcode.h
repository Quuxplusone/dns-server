#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

class Opcode {
public:
    enum detail : uint8_t {
        QUERY = 0,
        IQUERY = 1,
        STATUS = 2,
    };

    explicit constexpr Opcode(int x) : m_value(x) {}
    constexpr Opcode(enum Opcode::detail x) : m_value(int(x)) {}
    explicit constexpr operator int() const { return m_value; }
    constexpr bool operator==(Opcode rhs) const { return m_value == rhs.m_value; }
    constexpr bool operator!=(Opcode rhs) const { return m_value != rhs.m_value; }

    std::string repr() const {
        switch (m_value) {
            case QUERY: return "QUERY";
            case IQUERY: return "IQUERY";
            case STATUS: return "STATUS";
            default: return std::to_string(int(m_value));
        }
    }

private:
    uint8_t m_value;
};

} // namespace dns
