#pragma once

#include <string>

namespace dns {

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

/**
 *  Abstract class that represents the DNS Message and is able to code itself
 *  in the corresponding Message format.
 */
class Message {
public:
    /**
     *  Type of DNS message
     */
    enum Type {
        Query = 0,
        Response = 1,
    };

    /**
     *  Pure virtual function that will code the correspoding message type
     *  @param buffer The buffer to code the message into.
     *  @return The size of the buffer coded
     */
    virtual int code(char *buffer) noexcept = 0;

    /**
     *  Pure virtual function that will decode the correspoding message type
     *  @param buffer The buffer to decode the message into.
     *  @param size The size of the buffer to decode
     */
    virtual void decode(const char* buffer, int size) noexcept = 0;

    uint getID() const noexcept { return m_id; }
    uint getQdCount() const noexcept { return m_qdCount; }
    uint getAnCount() const noexcept { return m_anCount; }
    uint getNsCount() const noexcept { return m_nsCount; }
    uint getArCount() const noexcept { return m_arCount; }

    void setID(uint id) noexcept { m_id = id; }
    void setQdCount(uint count) noexcept { m_qdCount = count; }
    void setAnCount(uint count) noexcept { m_anCount = count; }
    void setNsCount(uint count) noexcept { m_nsCount = count; }
    void setArCount(uint count) noexcept { m_arCount = count; }

protected:
    static const uint HDR_OFFSET = 12;

    uint m_id;
    uint m_qr;
    uint m_opcode;
    uint m_aa;
    uint m_tc;
    uint m_rd;
    uint m_ra;
    uint m_rcode;

    uint m_qdCount;
    uint m_anCount;
    uint m_nsCount;
    uint m_arCount;

    /**
     *  Constructor.
     *  @param type The type of DNS Message
     */
    Message(Type type) : m_qr(type) { }

    /**
     *  Destructor
     */
    virtual ~Message() { }

    /**
     *  Returns the DNS message header as a string text.
     *  @return The string text with the header information.
     */
    virtual std::string asString() const noexcept;

    /**
     *  Function that decodes the DNS message header section.
     *  @param buffer The buffer to decode the message header from.
     */
    void decode_hdr(const char* buffer) noexcept;

    /**
     *  Function that codes the DNS message header section.
     *  @param buffer The buffer to code the message header into.
     */
    void code_hdr(char* buffer) noexcept;

    /**
     *  Helper function that get 16 bits from the buffer and keeps it an int.
     *  It helps in compatibility issues as ntohs()
     *  @param buffer The buffer to get the 16 bits from.
     *  @return An int holding the value extracted.
     */
    int get16bits(const char*& buffer) noexcept;

    /**
     *  Helper function that puts 16 bits into the buffer.
     *  It helps in compatibility issues as htons()
     *  @param buffer The buffer to put the 16 bits into.
     *  @param value An unsigned int holding the value to set the buffer.
     */
    void put16bits(char*& buffer, uint value) noexcept;

    /**
     *  Helper function that puts 32 bits into the buffer.
     *  It helps in compatibility issues as htonl()
     *  @param buffer The buffer to put the 32 bits into.
     *  @param value An unsigned long holding the value to set the buffer.
     */
    void put32bits(char*& buffer, ulong value) noexcept;

    /**
     *  Function that logs the whole buffer of a DNS Message
     *  @param buffer The buffer to be logged.
     *  @param size The size of the buffer.
     */
    void log_buffer(const char* buffer, int size) noexcept;

private:
    static const uint QR_MASK = 0x8000;
    static const uint OPCODE_MASK = 0x7800;
    static const uint AA_MASK = 0x0400;
    static const uint TC_MASK = 0x0200;
    static const uint RD_MASK = 0x0100;
    static const uint RA_MASK = 0x8000;
    static const uint RCODE_MASK = 0x000F;
};

} // namespace dns
