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

    void setName(Name name) { m_name = std::move(name); }

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
