

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

uint16_t decode_class_repr(const char *src, const char *end)
{
    if (end == src+2 && memcmp(src, "IN", 2) == 0) {
        return 1;
    } else {
        throw dns::UnsupportedException("Zonefile RR has a class other than IN");
    }
}

uint16_t decode_rrtype_repr(const char *src, const char *end)
{
    static const char *types[] = {
        nullptr, "A",     "NS",    nullptr, nullptr, "CNAME", "SOA",   "", // 0..7
        nullptr, nullptr, nullptr, nullptr, "PTR",   nullptr, nullptr, "MX", // 8..15
        "TXT", // 16
    };
    for (int i=0; i < sizeof types / sizeof types[0]; ++i) {
        if (types[i] == nullptr) continue;
        if (end - src == strlen(types[i]) && memcmp(src, types[i], end - src) == 0) {
            return i;
        }
    }
    throw dns::UnsupportedException("Zonefile RR has an unrecognized rrtype");
}

std::string decode_rdata_repr(uint16_t rrclass, uint16_t rrtype, const char *src, const char *end)
{
    if (rrclass == 1 && rrtype == RRType::A) {
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
    } else if (rrtype == RRType::CNAME) {
        Name canonical_name;
        src = canonical_name.decode_repr(src, end);
        if (src != end) throw dns::UnsupportedException("Zonefile RR of type CNAME has the wrong format");
        char buffer[255*64];
        char *buffer_end = canonical_name.encode(buffer, buffer + sizeof buffer);
        assert(buffer_end != nullptr);
        return std::string(buffer, buffer_end);
    } else if (rrclass == 1 && rrtype == RRType::MX) {
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
    } else {
        throw dns::UnsupportedException("Zonefile RR has unsupported type ", rrtype);
    }
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
    m_class = decode_class_repr(m[2].first, m[2].second);
    m_type = decode_rrtype_repr(m[3].first, m[3].second);
    m_rdata = decode_rdata_repr(m_class, m_type, m[4].first, m[4].second);
    return end;
}

static std::string class_mnemonic_for(uint16_t rrclass)
{
    if (rrclass == 1) return "IN";
    if (rrclass == 3) return "CH";
    return std::to_string(rrclass);
}

static std::string rrtype_mnemonic_for(uint16_t rrtype)
{
    static const char *types[] = {
        nullptr, "A",     "NS",    nullptr, nullptr, "CNAME", "SOA",   "", // 0..7
        nullptr, nullptr, nullptr, nullptr, "PTR",   nullptr, nullptr, "MX", // 8..15
        "TXT", // 16
    };
    if (rrtype < (sizeof types / sizeof types[0]) && types[rrtype] != nullptr) {
        return types[rrtype];
    } else {
        return std::to_string(rrtype);
    }
}

static std::string rdata_repr_for(uint16_t rrclass, uint16_t rrtype, const std::string& rdata)
{
    if (rrclass == 1 && rrtype == RRType::A) {
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
    } else if (rrtype == RRType::CNAME) {
        Name canonical_name;
        const char *end = rdata.data() + rdata.size();
        const char *src = canonical_name.decode(rdata.data(), end);
        assert(src == end);
        return canonical_name.repr();
    } else if (rrclass == 1 && rrtype == RRType::MX) {
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
    } else {
        throw dns::UnsupportedException("Zonefile RR has unsupported type ", rrtype);
    }
}

std::string RR::repr() const
{
    std::string result;
    result += m_name.repr();
    do { result += ' '; } while (result.size() < 32);
    do { result += ' '; } while ((result.size() % 8) != 0);
    result += std::to_string(m_ttl);
    do { result += ' '; } while ((result.size() % 8) != 0);
    result += class_mnemonic_for(m_class);
    result += ' ';
    result += rrtype_mnemonic_for(m_type);
    do { result += ' '; } while ((result.size() % 8) != 0);
    result += rdata_repr_for(m_class, m_type, m_rdata);
    return result;
}
