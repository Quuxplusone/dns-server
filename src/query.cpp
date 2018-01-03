
#include "logger.h"
#include "query.h"

#include <iostream>
#include <sstream>

using namespace dns;

void Query::decode(const char* buffer, int size) noexcept
{
    decode_hdr(buffer);
    buffer += HDR_OFFSET;

    decode_qname(buffer);

    m_qType = get16bits(buffer);
    m_qClass = get16bits(buffer);
}

void Query::decode_qname(const char*& buffer) noexcept
{
    m_qName.clear();
    int length = *buffer++;
    while (length != 0) {
        for (int i = 0; i < length; i++) {
            char c = *buffer++;
            m_qName.append(1, c);
        }
        length = *buffer++;
        if (length != 0) m_qName.append(1,'.');
    }
}
