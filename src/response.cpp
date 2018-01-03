
#include "logger.h"
#include "message.h"
#include "response.h"

#include <iostream>
#include <sstream>

using namespace dns;

int Response::encode(char *buffer) noexcept
{
    char *bufferBegin = buffer;

    encode_hdr(buffer);
    buffer += HDR_OFFSET;

    // Code Question section
    encode_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);

    // Code Answer section
    encode_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);
    put32bits(buffer, m_ttl);
    put16bits(buffer, m_rdLength);
    encode_domain(buffer, m_rdata);

    int size = buffer - bufferBegin;
    return size;
}

void Response::encode_domain(char *&buffer, const std::string& domain) noexcept
{
    int start = 0;
    int end;

    while ((end = domain.find('.', start)) != std::string::npos) {
        *buffer++ = end - start; // label length octet
        for (int i = start; i < end; ++i) {
            *buffer++ = domain[i]; // label octets
        }
        start = end + 1; // Skip '.'
    }

    *buffer++ = domain.size() - start; // last label length octet
    for (int i = start; i < domain.size(); ++i) {
        *buffer++ = domain[i]; // last label octets
    }

    *buffer++ = '\0';
}
