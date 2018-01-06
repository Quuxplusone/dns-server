#pragma once

#include "name.h"
#include "rrtype.h"

#include <inttypes.h>
#include <string>
#include <utility>

namespace dns {

/**
 *  Base class that represents a DNS resource record.
 */
class RR {
public:
    explicit RR() = default;
    explicit RR(Name name, RRType rrtype, RRClass rrclass, uint32_t ttl, std::string rdata) :
        m_name(std::move(name)), m_rrtype(rrtype), m_rrclass(rrclass), m_ttl(ttl), m_rdata(std::move(rdata)) {}

    const Name& name() const noexcept { return m_name; }
    uint16_t rrtype() const noexcept { return m_rrtype; }
    bool is_SOA_record() const noexcept { return m_rrtype == RRType::SOA; }
    bool is_NS_record() const noexcept { return m_rrtype == RRType::NS; }

    void set_name(Name name) { m_name = std::move(name); }

    /**
     *  Meaningful only for NS and CNAME records. Decode and return
     *  the single domain name encoded in this object's RDATA.
     *  @return The decoded NSDNAME or CNAME domain name.
     */
    Name rhs_name() const;

    const char *decode(const char *src, const char *end);
    char *encode(char *dst, const char *end) const noexcept;

    std::string repr() const;
    const char *decode_repr(const char *src, const char *end);

private:
    Name m_name;
    uint16_t m_rrtype;
    uint16_t m_rrclass;
    uint32_t m_ttl;
    std::string m_rdata;
};

} // namespace dns
