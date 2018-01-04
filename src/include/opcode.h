#pragma once

#include <inttypes.h>

namespace dns {

enum Opcode : uint8_t {
    QUERY = 0,
    IQUERY = 1,
    STATUS = 2,
};

} // namespace dns
