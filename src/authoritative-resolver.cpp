
#include "authoritative-resolver.h"
#include "exception.h"
#include "message.h"
#include "nonstd.h"
#include "question.h"
#include "rr.h"
#include "rrtype.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace dns;

AuthoritativeResolver::AuthoritativeResolver(const std::string& filename)
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

void AuthoritativeResolver::print_records() const
{
    struct Visitor {
        static void visit(const DomainTreeNode& node) {
            SymbolTable syms;  // empty, will not be used
            for (auto&& rr : node.m_rr_list) {
                std::cout << rr.repr(syms) << std::endl;
            }
            for (auto&& kv : node.m_children) {
                visit(kv.second);
            }
        }
    };
    Visitor v;
    v.visit(m_root);
}

void AuthoritativeResolver::add_SOA_to_authority_section(const DomainTreeNode *node, Message& response) const
{
    assert(node->is_top_of_zone());
    for (auto&& rr : node->m_rr_list) {
        if (rr.is_SOA_record()) {
            response.add_authority(rr);
            break;
        }
    }
}

void AuthoritativeResolver::populate_with_referral(const DomainTreeNode *zone_cut_node, Message& response) const
{
    for (auto&& rr : zone_cut_node->m_rr_list) {
        if (rr.is_NS_record()) {
            response.add_authority(rr);
            // TODO: add A and AAAA "glue" to the "additional" section
        }
    }
}

void AuthoritativeResolver::populate_response(const Question& question, Message& response) const
{
    response.add_question(Question(
        question.qname(),
        question.qtype(),
        question.qclass()
    ));

    const Name& name = question.qname();
    assert(name.labels().back().empty());

    const DomainTreeNode *node = &m_root;
    const DomainTreeNode *last_zone_cut_node = nullptr;
    const DomainTreeNode *last_top_of_zone_node = nullptr;
    bool in_authoritative_zone = false;
    bool found_nothing_in_tree = false;
    bool found_wildcard = false;
    for (auto&& label : nonstd::drop(1, nonstd::reversed(name.labels()))) {
        if (node->is_top_of_zone()) {
            in_authoritative_zone = true;
            last_top_of_zone_node = node;
        } else if (node->is_zone_cut()) {
            // RC 1034, section 4.2.1: the zone cut's NS records themselves are not authoritative
            in_authoritative_zone = false;
            last_zone_cut_node = node;
        }
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
        } else {
            found_nothing_in_tree = true;
        }
        break;
    }

    assert(node != nullptr);
    response.setAA(in_authoritative_zone);
    if (in_authoritative_zone) {
        if (found_nothing_in_tree) {
            response.setRCode(RCode::NXDOMAIN);
            add_SOA_to_authority_section(last_top_of_zone_node, response);
        } else {
            // We found either the qname, or a wildcard matching the qname.
            response.setRCode(RCode::NOERROR);
            for (auto&& rr : node->m_rr_list) {
                if (question.qtype() == rr.rrtype() || question.qtype() == RRType::ANY) {
                    // This RR is relevant!
                    RR modified_rr = rr;
                    if (found_wildcard) {
                        modified_rr.set_name(question.qname());
                    }
                    response.add_answer(std::move(modified_rr));
                }
            }
        }
    } else if (last_zone_cut_node != nullptr) {
        // RFC 1034, section 4.3.2, step 3b: respond with a referral
        // RFC 4592, section 4.2: it does not matter if the zone name in question is a wildcard
        response.setRCode(RCode::NOERROR);
        populate_with_referral(last_zone_cut_node, response);
    } else {
        // If we have no relevant authority at all, we should just refuse to answer.
        response.setRCode(RCode::REFUSED);
    }
}

void AuthoritativeResolver::add_rr(RR rr)
{
    bool is_SOA = rr.is_SOA_record();
    bool is_NS = rr.is_NS_record();
    DomainTreeNode *node = &m_root;
    const Name& name = rr.name();
    for (auto&& label : nonstd::drop(1, nonstd::reversed(name.labels()))) {
        node = &node->m_children[label];
    }
    node->m_rr_list.emplace_back(std::move(rr));
    if (is_SOA) node->m_has_SOA_record = true;
    if (is_NS) node->m_has_NS_record = true;
}
