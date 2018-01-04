
#include "query.h"

#include <string>

using namespace dns;

const char *Query::decode(const char *src, const char *end)
{
    src = decode_hdr(src, end);
    src = m_qName.decode(src, end);
    src = get16bits(src, end, m_qType);
    src = get16bits(src, end, m_qClass);
    return src;
}
