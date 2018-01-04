
#include "exception.h"
#include "message.h"
#include "question.h"
#include "resolver.h"
#include "rr.h"
#include "rrtype.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace dns;

Resolver::Resolver(const std::string& filename)
{
    std::ifstream file(filename.data());
    if (!file) {
        throw dns::Exception("Could not open file: ", filename);
    }

    std::string line;
    while (!file.eof()) {
        std::getline(file, line);
        if (line.empty()) continue;
        RR rr;
        const char *end = line.data() + line.size();
        const char *p = rr.decode_repr(line.data(), end);
        if (p != end) {
            throw dns::Exception("Zonefile RR contained trailing characters");
        }
        m_rr_list.emplace_back(std::move(rr));
    }
}

void Resolver::print_records() const
{
    for (auto&& rr : m_rr_list) {
        std::cout << rr.repr() << std::endl;
    }
}

Message Resolver::produce_response(const Question& question)
{
    Message response;
    response.add_question(Question(
        question.qname(),
        question.qtype(),
        question.qclass()
    ));
    response.setRCode(RCode::NXDOMAIN);

    for (auto&& rr : m_rr_list) {
        if (rr.getName() == question.qname()) {
            if (question.qtype() == rr.getType() || question.qtype() == RRType::ANY) {
                // This RR is relevant!
                response.setRCode(RCode::NOERROR);
                response.add_answer(rr);
            }
        }
    }
    return response;
}
