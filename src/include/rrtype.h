#pragma once

#include <inttypes.h>

namespace dns {

enum RRClass : uint16_t {
    IN = 1,
};

enum RRType : uint16_t {
    A = 1,
    NS = 2,
    CNAME = 5,
    SOA = 6,
    PTR = 12,
    MX = 15,
    TXT = 16,
    ANY = 255,
};

} // namespace dns
