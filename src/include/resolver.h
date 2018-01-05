#pragma once

#include "name.h"
#include "message.h"
#include "question.h"
#include "rr.h"

#include <list>
#include <map>
#include <string>

namespace dns {

class DomainTreeNode {
public:
    DomainTreeNode() = default;

    bool is_top_of_zone() const noexcept { return m_has_SOA_record; }
    bool is_zone_cut() const noexcept { return m_has_NS_record && !m_has_SOA_record; }

private:
    friend class Resolver;

    bool m_has_SOA_record = false;
    bool m_has_NS_record = false;
    std::map<Label, DomainTreeNode> m_children;
    std::list<RR> m_rr_list;
};

/**
 *  Resolver is the class that handles the @ref Query and resolves the domain
 *  names contained on it. It processes the @ref Query and set the appropiate
 *  values in the @ref Response.
 */
class Resolver {
public:
    /**
     *  Open the zonefile and read it to initialize the database.
     *  @param filename Name of the file containing the zone data.
     */
    explicit Resolver(const std::string& filename);

    /**
     *  Process the query and produce a response.
     *  @param question @ref Question that will be processed.
     */
    void populate_response(const Question& question, Message& response) const;

    /**
     *  Prints all records from the list.
     */
    void print_records() const;

private:
    void add_rr(RR rr);
    void add_SOA_to_authority_section(const DomainTreeNode *node, Message& response) const;
    void populate_with_referral(const DomainTreeNode *zone_cut_node, Message& response) const;

    DomainTreeNode m_root;
};

} // namespace dns
