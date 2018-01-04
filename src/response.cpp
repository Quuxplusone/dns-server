
#include "logger.h"
#include "message.h"
#include "response.h"

#include <iostream>
#include <sstream>

using namespace dns;

char *Response::encode(char *dst, const char *end) noexcept
{
    dst = encode_hdr(dst, end);

    // Encode Question section
    dst = m_name.encode(dst, end);
    dst = put16bits(dst, end, m_type);
    dst = put16bits(dst, end, m_class);

    // Encode Answer section
    dst = m_name.encode(dst, end);
    dst = put16bits(dst, end, m_type);
    dst = put16bits(dst, end, m_class);
    dst = put32bits(dst, end, m_ttl);
    dst = put16bits(dst, end, m_rdLength);
    dst = m_rdata.encode(dst, end);

    return dst;
}
