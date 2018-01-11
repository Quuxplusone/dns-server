#pragma once

#include "name.h"
#include "rrtype.h"
#include "symboltable.h"

#include <inttypes.h>
#include <utility>

namespace dns {

class Question {
public:
    explicit Question() = default;
    explicit Question(Name qname, RRType qtype, RRClass qclass) :
        m_qname(std::move(qname)), m_qtype(int(qtype)), m_qclass(int(qclass)) {}

    const Name& qname() const noexcept { return m_qname; }
    RRType qtype() const noexcept { return RRType(m_qtype); }
    RRClass qclass() const noexcept { return RRClass(m_qclass); }

    const char *decode(const SymbolTable& syms, const char *src, const char *end);
    char *encode(char *dst, const char *end) const noexcept;

    std::string repr() const;

private:
    Name m_qname;
    uint16_t m_qtype;
    uint16_t m_qclass;
};

} // namespace dns
