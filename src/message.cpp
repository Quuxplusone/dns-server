
#include "logger.h"
#include "message.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <netinet/in.h>

using namespace dns;

void Message::decode_hdr(const char *buffer) noexcept
{
    m_id = get16bits(buffer);

    uint fields = get16bits(buffer);
    m_qr = fields & QR_MASK;
    m_opcode = fields & OPCODE_MASK;
    m_aa = fields & AA_MASK;
    m_tc = fields & TC_MASK;
    m_rd = fields & RD_MASK;
    m_ra = fields & RA_MASK;

    m_qdCount = get16bits(buffer);
    m_anCount = get16bits(buffer);
    m_nsCount = get16bits(buffer);
    m_arCount = get16bits(buffer);
}

void Message::encode_hdr(char *buffer) noexcept
{
    put16bits(buffer, m_id);

    int fields = (m_qr << 15);
    fields += (m_opcode << 14);
    //...
    fields += m_rcode;
    put16bits(buffer, fields);

    put16bits(buffer, m_qdCount);
    put16bits(buffer, m_anCount);
    put16bits(buffer, m_nsCount);
    put16bits(buffer, m_arCount);
}

int Message::get16bits(const char*& buffer) noexcept
{
    int value = static_cast<uchar> (buffer[0]);
    value = value << 8;
    value += static_cast<uchar> (buffer[1]);
    buffer += 2;

    return value;
}

void Message::put16bits(char*& buffer, uint value) noexcept
{
    buffer[0] = (value & 0xFF00) >> 8;
    buffer[1] = value & 0xFF;
    buffer += 2;
}

void Message::put32bits(char*& buffer, ulong value) noexcept
{
    buffer[0] = (value & 0xFF000000) >> 24;
    buffer[1] = (value & 0xFF0000) >> 16;
    buffer[2] = (value & 0xFF00) >> 16;
    buffer[3] = (value & 0xFF) >> 16;
    buffer += 4;
}
