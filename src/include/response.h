#pragma once

#include "message.h"

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
     *  Function that codes the response message in its format.
     *  @param buffer The buffer to code the query into.
     *  @return The size of the buffer coded
     */
    int encode(char* buffer) noexcept;

    void setRCode(Code code) noexcept { m_rcode = code; }
    void setName(const std::string& value) noexcept { m_name = value; }
    void setType(const uint value) noexcept { m_type = value; }
    void setClass(const uint value) noexcept { m_class = value; }
    void setTtl(const uint value) noexcept { m_ttl = value; }
    void setRdLength(const uint value) noexcept { m_rdLength = value; }
    void setRdata(const std::string& value) noexcept { m_rdata = value; }

private:
    std::string m_name;
    uint m_type;
    uint m_class;
    ulong m_ttl;
    uint m_rdLength;
    std::string m_rdata;

    void encode_domain(char*& buffer, const std::string& domain) noexcept;
};

} // namespace dns
