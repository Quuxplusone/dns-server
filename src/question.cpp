
#include "bytes.h"
#include "question.h"
#include "rrtype.h"

#include <assert.h>

using namespace dns;

const char *Question::decode(const SymbolTable& syms, const char *src, const char *end)
{
    src = m_qname.decode(syms, src, end);
    src = get16bits(src, end, m_qtype);
    src = get16bits(src, end, m_qclass);
    return src;
}

char *Question::encode(char *dst, const char *end) const noexcept
{
    dst = m_qname.encode(dst, end);
    dst = put16bits(dst, end, m_qtype);
    dst = put16bits(dst, end, m_qclass);
    return dst;
}

std::string Question::repr() const
{
    std::string result;
    result += m_qname.repr();
    do { result += ' '; } while (result.size() < 32);
    do { result += ' '; } while ((result.size() % 8) != 0);
    result += "        ";
    assert(m_qclass == RRClass::IN);
    result += "IN";
    result += ' ';
    result += RRType(m_qtype).repr();
    return result;
}
