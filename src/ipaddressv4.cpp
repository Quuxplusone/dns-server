
#include "exception.h"
#include "ipaddressv4.h"

#include <regex>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace dns;

const char *IPAddressV4::decode_repr(const char *src, const char *end)
{
    std::regex rx("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
    std::cmatch m;
    bool success = std::regex_match(src, end, m, rx);
    if (!success) return nullptr;
    int quad1 = atoi(m[1].first);
    int quad2 = atoi(m[2].first);
    int quad3 = atoi(m[3].first);
    int quad4 = atoi(m[4].str().c_str());
    if (quad1 > 255 || quad2 > 255 || quad3 > 255 || quad4 > 255) {
        throw dns::UnsupportedException("IPv4 address contains byte values over 255");
    }
    m_bytes[0] = quad1;
    m_bytes[1] = quad2;
    m_bytes[2] = quad3;
    m_bytes[3] = quad4;
    return m[0].second;
}

std::string IPAddressV4::repr() const
{
    std::string result = std::to_string(m_bytes[0]) + '.';
    result += std::to_string(m_bytes[1]) + '.';
    result += std::to_string(m_bytes[2]) + '.';
    result += std::to_string(m_bytes[3]);
    return result;
}

char *IPAddressV4::encode(char *dst, const char *end) const noexcept
{
    if ((dst == nullptr) || (end - dst) < 4) return nullptr;
    memcpy(dst, m_bytes, 4);
    return dst + 4;
}

const char *IPAddressV4::decode(const char *src, const char *end)
{
    if ((src == nullptr) || (end - src) < 4) return nullptr;
    memcpy(m_bytes, src, 4);
    return src + 4;
}
