
#include "exception.h"
#include "rrtype.h"

#include <regex>
#include <stdlib.h>
#include <string>

namespace dns {

RRClass::RRClass(const std::string& repr)
{
    for (int i=0; i <= ANY; ++i) {
        RRClass rrclass(i);
        if (repr == rrclass.repr()) {
            *this = rrclass;
            return;
        }
    }
    std::smatch m;
    if (std::regex_match(repr, m, std::regex("CLASS(\\d+)$"))) {
        int i = atoi(&*m[1].first);
        if (1 <= i && i <= 65535) {
            *this = RRClass(i);
            return;
        }
    }
    throw dns::UnsupportedException("rrclass ", repr, " is unknown or invalid");
}

RRType::RRType(const std::string& repr)
{
    for (int i=0; i <= ANY; ++i) {
        RRType type(i);
        if (repr == type.repr()) {
            *this = type;
            return;
        }
    }
    std::smatch m;
    if (std::regex_match(repr, m, std::regex("TYPE(\\d+)$"))) {
        int i = atoi(&*m[1].first);
        if (1 <= i && i <= 65535) {
            *this = RRType(i);
            return;
        }
    }
    throw dns::UnsupportedException("rrtype ", repr, " is unknown or invalid");
}

} // namespace dns
