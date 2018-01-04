#pragma once

#include <inttypes.h>

namespace dns {

enum RCode : uint8_t {
    NOERROR = 0,
    FORMERR = 1,
    SERVFAIL = 2,
    NXDOMAIN = 3,
    NOTIMP = 4,
    REFUSED = 5,
};

} // namespace dns
