#pragma once

#include "opcode.h"
#include "question.h"
#include "rcode.h"
#include "rr.h"
#include "symboltable.h"

#include <inttypes.h>
#include <string>

namespace dns {

/**
 *  Class that represents a DNS message (a query or a response).
 */
class Message {
public:
    explicit Message() = default;

    bool is_query() const noexcept { return !m_qr; }
    bool is_response() const noexcept { return m_qr; }
    const std::vector<Question>& questions() const noexcept { return m_question; }

    void setInResponseTo(const Message& q) noexcept;
    void setRCode(RCode rcode) noexcept { m_rcode = rcode; }
    void setAA(bool aa) noexcept { m_aa = aa; }
    void add_question(Question q) { m_question.emplace_back(std::move(q)); }
    void add_answer(RR rr) { m_answer.emplace_back(std::move(rr)); }
    void add_authority(RR rr) { m_authority.emplace_back(std::move(rr)); }
    void add_additional(RR rr) { m_additional.emplace_back(std::move(rr)); }

    /**
     *  Function that decodes a DNS message.
     *  @param src The input data from which to decode the message.
     *  @param end A pointer one past the end of the input data.
     *  @return A pointer one past the end of the encoded representation.
     */
    const char *decode(const char *packet_start, const char *end);

    /**
     *  Function that encodes a DNS message.
     *  @param dst The buffer into which to encode the message.
     *  @param end A pointer one past the end of the buffer.
     *  @return A pointer one past the end of the encoded representation.
     */
    char *encode(char *dst, const char *end) const noexcept;

    std::string repr() const;

private:
    uint16_t m_id = 0;
    bool m_qr = false;
    Opcode m_opcode = Opcode::QUERY;
    bool m_aa = false;
    bool m_tc = false;
    bool m_rd = false;
    bool m_ra = false;
    RCode m_rcode = RCode::NOERROR;

    SymbolTable m_symbol_table;
    std::vector<Question> m_question;
    std::vector<RR> m_answer;
    std::vector<RR> m_authority;
    std::vector<RR> m_additional;
};

} // namespace dns
