
#include "bytes.h"
#include "rr.h"

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
