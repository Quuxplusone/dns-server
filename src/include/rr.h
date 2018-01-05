#pragma once

#include "name.h"

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
    explicit RR(Name qname, uint16_t qtype, uint16_t qclass, uint32_t ttl, std::string rdata) :
        m_name(std::move(qname)), m_type(qtype), m_class(qclass), m_ttl(ttl), m_rdata(std::move(rdata)) {}

    const Name& getName() const noexcept { return m_name; }
    uint16_t getType() const noexcept { return m_type; }
    uint16_t getClass() const noexcept { return m_class; }

    void setName(Name name) { m_name = std::move(name); }

    const char *decode(const char *src, const char *end);
    char *encode(char *dst, const char *end) const noexcept;

    std::string repr() const;
    const char *decode_repr(const char *src, const char *end);

private:
    Name m_name;
    uint16_t m_type;
    uint16_t m_class;
    uint32_t m_ttl;
    std::string m_rdata;
};

} // namespace dns
