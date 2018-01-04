#pragma once

#include "message.h"
#include "name.h"

namespace dns {

/**
 *  Class that represents the DNS Response and is able to code itself in its
 *  corresponding message format.
 */
class Response : public Message {
public:
    enum Code {
        NOERROR = 0,
        FORMERR = 1,
        SERVFAIL = 2,
        NXDOMAIN = 3,
        NOTIMP = 4,
        REFUSED = 5,
    };

    Response() : Message(Message::Response) {}

    /**
     *  Function that encodes the response message into a packet.
     *  @param buffer The buffer into which to encode the message.
     *  @param end A pointer one past the end of the buffer.
     *  @return A pointer one past the end of the encoded representation.
     */
    char *encode(char *dst, const char *end) noexcept;

    void setRCode(Code code) noexcept { m_rcode = code; }
    void setName(const Name& value) noexcept { m_name = value; }
    void setType(uint16_t value) noexcept { m_type = value; }
    void setClass(uint16_t value) noexcept { m_class = value; }
    void setTtl(uint32_t value) noexcept { m_ttl = value; }
    void setRdLength(uint16_t value) noexcept { m_rdLength = value; }
    void setRdata(const Name& value) noexcept { m_rdata = value; }

private:
    Name m_name;
    uint16_t m_type;
    uint16_t m_class;
    uint32_t m_ttl;
    uint16_t m_rdLength;
    Name m_rdata;
};

} // namespace dns
