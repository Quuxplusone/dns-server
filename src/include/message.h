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

    static Message beginQuery(Question question) noexcept;
    static Message beginResponseTo(const Message& query) noexcept;

    bool is_query() const noexcept { return !m_qr; }
    bool is_response() const noexcept { return m_qr; }
    Opcode opcode() const noexcept { return m_opcode; }
    const std::vector<Question>& questions() const noexcept { return m_question; }
    const std::vector<RR>& answers() const noexcept { return m_answer; }
    const std::vector<RR>& authority() const noexcept { return m_authority; }
    const std::vector<RR>& additional() const noexcept { return m_additional; }

    Message& setID(uint16_t id) noexcept { m_id = id; return *this; }
    Message& setOpcode(Opcode opcode) noexcept { m_opcode = opcode; return *this; }
    Message& setRCode(RCode rcode) noexcept { m_rcode = rcode; return *this; }
    Message& setQR(bool qr) noexcept { m_qr = qr; return *this; }
    Message& setAA(bool aa) noexcept { m_aa = aa; return *this; }
    Message& setRD(bool rd) noexcept { m_rd = rd; return *this; }
    Message& setRA(bool ra) noexcept { m_ra = ra; return *this; }
    Message& add_question(Question q) { m_question.emplace_back(std::move(q)); return *this; }
    Message& add_answer(RR rr) { m_answer.emplace_back(std::move(rr)); return *this; }
    Message& add_authority(RR rr) { m_authority.emplace_back(std::move(rr)); return *this; }
    Message& add_additional(RR rr) { m_additional.emplace_back(std::move(rr)); return *this; }

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
