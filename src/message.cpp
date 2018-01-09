
#include "bytes.h"
#include "message.h"
#include "opcode.h"
#include "rcode.h"

using namespace dns;

void Message::setInResponseTo(const Message& q) noexcept
{
    m_id = q.m_id;
    m_qr = true;
    m_opcode = q.m_opcode;
    m_aa = true;
    m_rd = q.m_rd;
    m_ra = false;
}

const char *Message::decode(const char *packet_start, const char *end)
{
    const char *src = packet_start;

    uint16_t fields;
    uint16_t qdcount, ancount, nscount, arcount;

    src = get16bits(src, end, m_id);
    src = get16bits(src, end, fields);
    src = get16bits(src, end, qdcount);
    src = get16bits(src, end, ancount);
    src = get16bits(src, end, nscount);
    src = get16bits(src, end, arcount);

    if (src == nullptr) return nullptr;

    m_qr = ((fields >> 15) & 0x1);
    m_opcode = static_cast<Opcode>((fields >> 11) & 0xF);
    m_aa = ((fields >> 10) & 0x1);
    m_tc = ((fields >> 9) & 0x1);
    m_rd = ((fields >> 8) & 0x1);
    m_ra = ((fields >> 7) & 0x1);
    m_rcode = static_cast<RCode>((fields >> 0) & 0x4);

    // Now, before parsing any other names, build a "symbol table"
    // consisting of all the names that could possibly be encoded
    // in this packet's bytes.
    m_symbol_table.build(packet_start, end);

    for (uint16_t i=0; i < qdcount; ++i) {
        m_question.emplace_back();
        src = m_question.back().decode(m_symbol_table, src, end);
        if (src == nullptr) return nullptr;
    }
    for (uint16_t i=0; i < ancount; ++i) {
        m_answer.emplace_back();
        src = m_answer.back().decode(m_symbol_table, src, end);
        if (src == nullptr) return nullptr;
    }
    for (uint16_t i=0; i < nscount; ++i) {
        m_authority.emplace_back();
        src = m_authority.back().decode(m_symbol_table, src, end);
        if (src == nullptr) return nullptr;
    }
    for (uint16_t i=0; i < arcount; ++i) {
        m_additional.emplace_back();
        src = m_additional.back().decode(m_symbol_table, src, end);
        if (src == nullptr) return nullptr;
    }
    return src;
}

char *Message::encode(char *dst, const char *end) const noexcept
{
    int fields = (int(m_qr) << 15);
    fields |= (int(m_opcode) << 14);
    fields |= (int(m_aa) << 10);
    fields |= (int(m_tc) << 9);
    fields |= (int(m_rd) << 8);
    fields |= (int(m_ra) << 7);
    fields |= (int(m_rcode) << 0);

    dst = put16bits(dst, end, m_id);
    dst = put16bits(dst, end, fields);
    dst = put16bits(dst, end, m_question.size());
    dst = put16bits(dst, end, m_answer.size());
    dst = put16bits(dst, end, m_authority.size());
    dst = put16bits(dst, end, m_additional.size());

    for (auto&& q : m_question) {
        dst = q.encode(dst, end);
    }
    for (auto&& rr : m_answer) {
        dst = rr.encode(dst, end);
    }
    for (auto&& rr : m_authority) {
        dst = rr.encode(dst, end);
    }
    for (auto&& rr : m_additional) {
        dst = rr.encode(dst, end);
    }
    return dst;
}
