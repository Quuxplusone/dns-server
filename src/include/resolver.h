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

private:
    friend class Resolver;

    bool m_is_nxdomain = true;
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
    void populate_response(const Question& question, Message& response);

    /**
     *  Prints all records from the list.
     */
    void print_records() const;

private:
    void add_rr(RR rr);

    DomainTreeNode m_root;
};

} // namespace dns
