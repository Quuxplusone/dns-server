#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

/**
 *  Class that represents an IPv4 (32-bit) address.
 */
class IPAddressV4 {
public:
    explicit IPAddressV4() = default;

    const char *decode(const char *src, const char *end);
    char *encode(char *dst, const char *end) const noexcept;

    std::string repr() const;
    const char *decode_repr(const char *src, const char *end);

private:
    uint8_t m_bytes[4];  // e.g. {127, 0, 0, 1}
};

} // namespace dns
