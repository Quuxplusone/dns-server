#pragma once

#include "name.h"

#include <inttypes.h>
#include <utility>

namespace dns {

class Question {
public:
    explicit Question() = default;
    explicit Question(Name qname, uint16_t qtype, uint16_t qclass) :
        m_qname(std::move(qname)), m_qtype(qtype), m_qclass(qclass) {}

    const Name& qname() const noexcept { return m_qname; }
    uint16_t qtype() const noexcept { return m_qtype; }
    uint16_t qclass() const noexcept { return m_qclass; }

    const char *decode(const SymbolTable& syms, const char *src, const char *end);
    char *encode(char *dst, const char *end) const noexcept;

private:
    Name m_qname;
    uint16_t m_qtype;
    uint16_t m_qclass;
};

} // namespace dns
