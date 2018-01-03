#pragma once

#include <string>

#include "message.h"

namespace dns {

/**
 *  Class that represents the DNS Query and is able to code itself in its
 *  corresponding message format.
 */
class Query : public Message {
public:
    Query() : Message(Message::Query) { }

    /**
     *  Function that codes the query message in its format.
     *  @param buffer The buffer to code the query into.
     *  @return The size of the buffer coded
     */
    int encode(char* buffer) noexcept;

    /**
     *  Function that decodes the query message in its format.
     *  @param buffer The buffer to decode the query into.
     *  @param size The size of the buffer to decode
     */
    void decode(const char* buffer, int size) noexcept;

    /**
     *  Returns the query message as a string text.
     *  @return The string text with the query information.
     */
    std::string asString() const noexcept;

    const std::string& getQName() const noexcept { return m_qName; }
    const uint getQType() const noexcept { return m_qType; }
    const uint getQClass() const noexcept { return m_qClass; }

private:
    std::string m_qName;
    uint m_qType;
    uint m_qClass;

    void decode_qname(const char*& buffer) noexcept;
};

} // namespace dns
