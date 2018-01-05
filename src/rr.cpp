

#include "bytes.h"
#include "exception.h"
#include "rr.h"
#include "rrtype.h"

#include <assert.h>
#include <regex>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace dns;

static std::string encode_rdata_repr_just_domain_name(const std::string& rdata);

template<RRType rrtype>
static std::string decode_rdata_repr_just_domain_name(const char *src, const char *end);

struct by_rrtype_t {
    RRType type;
    const char *str;
    std::string (*encode_rdata_repr)(const std::string& rdata);
    std::string (*decode_rdata_repr)(const char *src, const char *end);
};

static by_rrtype_t by_rrtype[17] = {
    {},
    {
        RRType::A, "A",
        [](const std::string& rdata) -> std::string {
            assert(rdata.size() == 4);
            std::string result;
            result += std::to_string(uint8_t(rdata[0]));
            result += '.';
            result += std::to_string(uint8_t(rdata[1]));
            result += '.';
            result += std::to_string(uint8_t(rdata[2]));
            result += '.';
            result += std::to_string(uint8_t(rdata[3]));
            return result;
        },
        [](const char *src, const char *end) -> std::string {
            std::regex rx("(\\d+)[.](\\d+)[.](\\d+)[.](\\d+)$");
            std::cmatch m;
            bool success = std::regex_match(src, end, m, rx);
            if (!success) {
                throw dns::UnsupportedException("Zonefile RR of type A has a malformed IP address");
            }
            int quad1 = atoi(m[1].first);
            int quad2 = atoi(m[2].first);
            int quad3 = atoi(m[3].first);
            int quad4 = atoi(m[4].first);
            if (quad1 > 255 || quad2 > 255 || quad3 > 255 || quad4 > 255) {
                throw dns::UnsupportedException("Zonefile RR of type A has a malformed IP address");
            }
            std::string result(4, '\0');
            result[0] = quad1;
            result[1] = quad2;
            result[2] = quad3;
            result[3] = quad4;
            return result;
        },
    },
    {
        RRType::NS, "NS",
        encode_rdata_repr_just_domain_name,
        decode_rdata_repr_just_domain_name<RRType::NS>,
    },
    {},
    {},
    {
        RRType::CNAME, "CNAME",
        encode_rdata_repr_just_domain_name,
        decode_rdata_repr_just_domain_name<RRType::CNAME>,
    },
    {
        RRType::SOA, "SOA",
        nullptr,
        nullptr,
    },
    {},
    {},
    {},
    {},
    {},
    {
        RRType::PTR, "PTR",
        encode_rdata_repr_just_domain_name,
        decode_rdata_repr_just_domain_name<RRType::PTR>,
    },
    {},
    {},
    {
        RRType::MX, "MX",
        [](const std::string& rdata) -> std::string {
            Name exchange_name;
            const char *end = rdata.data() + rdata.size();
            uint16_t preference;
            const char *src = get16bits(rdata.data(), end, preference);
            src = exchange_name.decode(src, end);
            assert(src == end);
            std::string result = std::to_string(preference);
            do { result += ' '; } while ((result.size() % 8) != 0);
            result += exchange_name.repr();
            return result;
        },
        [](const char *src, const char *end) -> std::string {
            std::regex rx("(\\d+)\\s+(.*)$");
            std::cmatch m;
            bool success = std::regex_match(src, end, m, rx);
            if (!success) {
                throw dns::UnsupportedException("Zonefile RR of type MX has the wrong format");
            }
            int preference = atoi(m[1].first);
            if (preference < 0 || 65535 < preference) throw dns::UnsupportedException("Zonefile RR of type MX has an out-of-range PREFERENCE");
            Name exchange_name;
            src = exchange_name.decode_repr(m[2].first, m[2].second);
            if (src != end) throw dns::UnsupportedException("Zonefile RR of type MX has the wrong format");
            char buffer[255*64 + 2];
            char *p = put16bits(buffer, buffer + sizeof buffer, preference);
            char *buffer_end = exchange_name.encode(p, buffer + sizeof buffer);
            assert(buffer_end != nullptr);
            return std::string(buffer, buffer_end);
        },
    },
    {
        RRType::TXT, "TXT",
        nullptr,
        nullptr,
    },
};

static std::string encode_rdata_repr_just_domain_name(const std::string& rdata)
{
    Name canonical_name;
    const char *end = rdata.data() + rdata.size();
    const char *src = canonical_name.decode(rdata.data(), end);
    assert(src == end);
    return canonical_name.repr();
}

template<RRType rrtype>
static std::string decode_rdata_repr_just_domain_name(const char *src, const char *end)
{
    assert(by_rrtype[rrtype].str != nullptr);
    Name canonical_name;
    src = canonical_name.decode_repr(src, end);
    if (src != end) throw dns::UnsupportedException("Zonefile RR of type ", by_rrtype[rrtype].str, " has the wrong format");
    char buffer[255*64];
    char *buffer_end = canonical_name.encode(buffer, buffer + sizeof buffer);
    assert(buffer_end != nullptr);
    return std::string(buffer, buffer_end);
}

const char *RR::decode(const char *src, const char *end)
{
    src = m_name.decode(src, end);
    src = get16bits(src, end, m_type);
    src = get16bits(src, end, m_class);
    src = get32bits(src, end, m_ttl);
    src = get_uint16_sized_string(src, end, m_rdata);
    return src;
}

char *RR::encode(char *dst, const char *end) const noexcept
{
    dst = m_name.encode(dst, end);
    dst = put16bits(dst, end, m_type);
    dst = put16bits(dst, end, m_class);
    dst = put32bits(dst, end, m_ttl);
    dst = put_uint16_sized_string(dst, end, m_rdata);
    return dst;
}

const char *RR::decode_repr(const char *src, const char *end)
{
    if (src == nullptr) return nullptr;
    while (src != end && isspace(*src)) ++src;
    src = m_name.decode_repr(src, end);
    if (src == nullptr) return nullptr;
    std::regex rx("\\s*(\\d+)\\s+(\\S+)\\s+(\\S+)\\s+(.*?)\\s*$");
    std::cmatch m;
    bool success = std::regex_match(src, end, m, rx);
    if (!success) {
        throw dns::UnsupportedException("Zonefile RR has the wrong format");
    }
    int ttl = atoi(m[1].first);
    if (1 <= ttl && ttl <= 999999999) {
        m_ttl = ttl;
    } else {
        throw dns::UnsupportedException("Zonefile RR has an out-of-range TTL");
    }
    if (m[2].compare("IN") == 0) {
        m_class = 1;  // IN
    } else {
        throw dns::UnsupportedException("Zonefile RR has a class other than IN");
    }
    success = false;
    for (auto&& rrt : by_rrtype) {
        if (rrt.str != nullptr && m[3].compare(rrt.str) == 0) {
            m_type = rrt.type;
            m_rdata = rrt.decode_rdata_repr(m[4].first, m[4].second);
            success = true;
            break;
        }
    }
    if (!success) {
        throw dns::UnsupportedException("Zonefile RR has unsupported type ", m[3].str());
    }
    return end;
}

std::string RR::repr() const
{
    std::string result;
    result += m_name.repr();
    do { result += ' '; } while (result.size() < 32);
    do { result += ' '; } while ((result.size() % 8) != 0);
    result += std::to_string(m_ttl);
    do { result += ' '; } while ((result.size() % 8) != 0);
    assert(m_class == 1);
    result += "IN";
    result += ' ';
    bool success = false;
    for (auto&& rrt : by_rrtype) {
        if (rrt.str != nullptr && rrt.type == m_type) {
            result += rrt.str;
            do { result += ' '; } while ((result.size() % 8) != 0);
            result += rrt.encode_rdata_repr(m_rdata);
            success = true;
            break;
        }
    }
    assert(success);
    return result;
}
