
#include "logger.h"
#include "message.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <netinet/in.h>

using namespace dns;

const char *Message::decode_hdr(const char *src, const char *end) noexcept
{
    src = get16bits(src, end, m_id);

    uint16_t fields = 0;
    src = get16bits(src, end, fields);
    m_qr = fields & QR_MASK;
    m_opcode = fields & OPCODE_MASK;
    m_aa = fields & AA_MASK;
    m_tc = fields & TC_MASK;
    m_rd = fields & RD_MASK;
    m_ra = fields & RA_MASK;

    src = get16bits(src, end, m_qdCount);
    src = get16bits(src, end, m_anCount);
    src = get16bits(src, end, m_nsCount);
    src = get16bits(src, end, m_arCount);
    return src;
}

char *Message::encode_hdr(char *dst, const char *end) noexcept
{
    int fields = (m_qr << 15);
    fields |= (m_opcode << 14);
    fields |= (m_aa << 10);
    fields |= (m_tc << 9);
    fields |= (m_rd << 8);
    fields |= (m_ra << 7);
    fields |= (m_rcode << 0);

    dst = put16bits(dst, end, m_id);
    dst = put16bits(dst, end, fields);
    dst = put16bits(dst, end, m_qdCount);
    dst = put16bits(dst, end, m_anCount);
    dst = put16bits(dst, end, m_nsCount);
    dst = put16bits(dst, end, m_arCount);
    return dst;
}

const char *Message::get16bits(const char *src, const char *end, uint16_t& value) noexcept
{
    if (src == nullptr || src == end || src+1 == end) {
        value = 0;
        return nullptr;
    }
    value = (static_cast<uint8_t>(src[0]) << 8) | static_cast<uint8_t>(src[1]);
    src += 2;
    return src;
}

char *Message::put16bits(char *dst, const char *end, uint16_t value) noexcept
{
    if (dst == nullptr || dst == end || dst+1 == end) {
        return nullptr;
    }
    *dst++ = static_cast<uint8_t>(value >> 8);
    *dst++ = static_cast<uint8_t>(value >> 0);
    return dst;
}

char *Message::put32bits(char *dst, const char *end, uint32_t value) noexcept
{
    if (dst == nullptr || dst == end || dst+1 == end || dst+2 == end || dst+3 == end) {
        return nullptr;
    }
    *dst++ = static_cast<uint8_t>(value >> 24);
    *dst++ = static_cast<uint8_t>(value >> 16);
    *dst++ = static_cast<uint8_t>(value >> 8);
    *dst++ = static_cast<uint8_t>(value >> 0);
    return dst;
}
