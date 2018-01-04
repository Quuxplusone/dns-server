#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

/**
 *  Base class that represents the shared header of a DNS query or response.
 */
class Message {
public:
    enum QR {
        Query = 0,
        Response = 1,
    };

    uint16_t getID() const noexcept { return m_id; }
    uint16_t getQdCount() const noexcept { return m_qdCount; }
    uint16_t getAnCount() const noexcept { return m_anCount; }
    uint16_t getNsCount() const noexcept { return m_nsCount; }
    uint16_t getArCount() const noexcept { return m_arCount; }

    void setID(uint16_t id) noexcept { m_id = id; }
    void setQdCount(uint16_t count) noexcept { m_qdCount = count; }
    void setAnCount(uint16_t count) noexcept { m_anCount = count; }
    void setNsCount(uint16_t count) noexcept { m_nsCount = count; }
    void setArCount(uint16_t count) noexcept { m_arCount = count; }

protected:
    uint16_t m_id;
    uint16_t m_qr;
    uint16_t m_opcode;
    uint16_t m_aa;
    uint16_t m_tc;
    uint16_t m_rd;
    uint16_t m_ra;
    uint16_t m_rcode;

    uint16_t m_qdCount;
    uint16_t m_anCount;
    uint16_t m_nsCount;
    uint16_t m_arCount;

    explicit Message(QR qr) noexcept : m_qr(qr) {}

    /**
     *  Function that decodes the DNS message header section.
     *  @param src The input data from which to decode the message header.
     *  @param end A pointer one past the end of the input data.
     *  @return A pointer one past the end of the encoded representation.
     */
    const char *decode_hdr(const char *src, const char *end) noexcept;

    /**
     *  Function that codes the DNS message header section.
     *  @param dst The buffer to code the message header into.
     *  @param end A pointer one past the end of the buffer.
     *  @return A pointer one past the end of the encoded representation.
     */
    char *encode_hdr(char *dst, const char *end) noexcept;

    /**
     *  Helper function that get 16 bits from the buffer and keeps it an int.
     *  It helps in compatibility issues as ntohs()
     *  @param buffer The buffer to get the 16 bits from.
     *  @return An int holding the value extracted.
     */
    const char *get16bits(const char *src, const char *end, uint16_t& value) noexcept;

    /**
     *  Helper function that puts 16 bits into the buffer.
     *  It helps in compatibility issues as htons()
     *  @param buffer The buffer to put the 16 bits into.
     *  @param value An unsigned int holding the value to set the buffer.
     */
    char *put16bits(char *dst, const char *end, uint16_t value) noexcept;

    /**
     *  Helper function that puts 32 bits into the buffer.
     *  It helps in compatibility issues as htonl()
     *  @param buffer The buffer to put the 32 bits into.
     *  @param value An unsigned long holding the value to set the buffer.
     */
    char *put32bits(char *dst, const char *end, uint32_t value) noexcept;

private:
    static const uint16_t QR_MASK = 0x8000;
    static const uint16_t OPCODE_MASK = 0x7800;
    static const uint16_t AA_MASK = 0x0400;
    static const uint16_t TC_MASK = 0x0200;
    static const uint16_t RD_MASK = 0x0100;
    static const uint16_t RA_MASK = 0x8000;
    static const uint16_t RCODE_MASK = 0x000F;
};

} // namespace dns
