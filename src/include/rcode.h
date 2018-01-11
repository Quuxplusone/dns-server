#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

class RCode {
public:
    enum detail : uint8_t {
        NOERROR = 0,
        FORMERR = 1,
        SERVFAIL = 2,
        NXDOMAIN = 3,
        NOTIMP = 4,
        REFUSED = 5,
    };

    explicit constexpr RCode() = default;
    explicit constexpr RCode(int x) : m_value(x) {}
    constexpr RCode(enum RCode::detail x) : m_value(int(x)) {}
    explicit constexpr operator int() const { return m_value; }
    constexpr bool operator==(RCode rhs) const { return m_value == rhs.m_value; }
    constexpr bool operator!=(RCode rhs) const { return m_value != rhs.m_value; }

    std::string repr() const {
        switch (m_value) {
            case NOERROR: return "NOERROR";
            case FORMERR: return "FORMERR";
            case SERVFAIL: return "SERVFAIL";
            case NXDOMAIN: return "NXDOMAIN";
            case NOTIMP: return "NOTIMP";
            case REFUSED: return "REFUSED";
            default: return std::to_string(int(m_value));
        }
    }

private:
    uint8_t m_value = 0;
};

} // namespace dns
