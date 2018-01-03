
#include "logger.h"
#include "query.h"

#include <iostream>
#include <sstream>

using namespace dns;

std::string Query::asString() const noexcept
{
    std::ostringstream text;
    text << std::endl << "QUERY { ";
    text << Message::asString();
    text << "\tQname: " << m_qName << std::endl;
    text << "\tQtype: " << m_qType << std::endl;
    text << "\tQclass: " << m_qClass;
    text << " }" << std::dec;
    return text.str();
}

int Query::encode(char* buffer) noexcept
{
    // Only needed for the DNS client
    return 0;
}

void Query::decode(const char* buffer, int size) noexcept
{
    Logger::trace("Query::decode()");
    log_buffer(buffer, size);

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
