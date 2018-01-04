#pragma once

#include <string>

#include "message.h"
#include "name.h"

namespace dns {

/**
 *  Class that represents the DNS Query and is able to code itself in its
 *  corresponding message format.
 */
class Query : public Message {
public:
    Query() : Message(Message::Query) {}

    /**
     *  Function that decodes a packet into a query message.
     *  @param src The input data from which to decode the query.
     *  @param end A pointer one past the end of the input data.
     *  @return A pointer one past the end of the encoded representation.
     */
    const char *decode(const char *src, const char *end);

    const Name& getQName() const noexcept { return m_qName; }
    uint16_t getQType() const noexcept { return m_qType; }
    uint16_t getQClass() const noexcept { return m_qClass; }

private:
    Name m_qName;
    uint16_t m_qType;
    uint16_t m_qClass;
};

} // namespace dns
