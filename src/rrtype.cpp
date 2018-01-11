
#include "exception.h"
#include "rrtype.h"

#include <string>

namespace dns {

RRType::RRType(const std::string& repr)
{
    for (int i=0; i <= ANY; ++i) {
        RRType type(i);
        if (repr == type.repr()) {
            *this = type;
            return;
        }
    }
    throw dns::UnsupportedException("rrtype ", repr, " is unknown or invalid");
}

} // namespace dns
