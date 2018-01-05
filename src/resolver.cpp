
#include "exception.h"
#include "message.h"
#include "nonstd.h"
#include "question.h"
#include "resolver.h"
#include "rr.h"
#include "rrtype.h"

#include <assert.h>
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
        add_rr(std::move(rr));
    }
}

void Resolver::print_records() const
{
    struct Visitor {
        static void visit(const DomainTreeNode& node) {
            for (auto&& rr : node.m_rr_list) {
                std::cout << rr.repr() << std::endl;
            }
            for (auto&& kv : node.m_children) {
                visit(kv.second);
            }
        }
    };
    Visitor v;
    v.visit(m_root);
}

void Resolver::populate_response(const Question& question, Message& response)
{
    response.add_question(Question(
        question.qname(),
        question.qtype(),
        question.qclass()
    ));

    const Name& name = question.qname();
    assert(name.labels().back().empty());

    DomainTreeNode *node = &m_root;
    bool found_nxdomain = false;
    bool found_wildcard = false;
    for (auto&& label : nonstd::drop(1, nonstd::reversed(name.labels()))) {
        // RFC 1034, section 4.3.2, step 3
        auto it = node->m_children.find(label);
        if (it != node->m_children.end()) {
            node = &it->second;
            continue;
        }
        // A match is impossible. Step 3c.
        auto star = node->m_children.find(Label::asterisk());
        if (star != node->m_children.end()) {
            node = &star->second;
            found_wildcard = true;
            break;
        } else {
            found_nxdomain = true;
            break;
        }
    }

    if (found_nxdomain) {
        response.setRCode(RCode::NXDOMAIN);
    } else if (node->m_is_nxdomain) {
        response.setRCode(RCode::NXDOMAIN);
    } else {
        assert(node != nullptr);
        response.setRCode(RCode::NOERROR);
        for (auto&& rr : node->m_rr_list) {
            if (question.qtype() == rr.getType() || question.qtype() == RRType::ANY) {
                // This RR is relevant!
                RR modified_rr = rr;
                if (found_wildcard) {
                    modified_rr.setName(question.qname());
                }
                response.add_answer(std::move(modified_rr));
            }
        }
    }
}

void Resolver::add_rr(RR rr)
{
    DomainTreeNode *node = &m_root;
    const Name& name = rr.getName();
    for (auto&& label : nonstd::drop(1, nonstd::reversed(name.labels()))) {
        node = &node->m_children[label];
    }
    node->m_rr_list.emplace_back(std::move(rr));
    node->m_is_nxdomain = false;
}
