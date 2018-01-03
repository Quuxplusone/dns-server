#pragma once

#include "exception.h"

#include <list>
#include <string>
#include <utility>

namespace dns {

class Query;
class Response;

/**
 *  Resolver is the class that handles the @ref Query and resolves the domain
 *  names contained on it. It processes the @ref Query and set the appropiate
 *  values in the @ref Response.
 */
class Resolver {
public:
    Resolver() = default;

    /**
     *  Open the hosts file and read it to stores the ipAddress-hostname pairs.
     *  @param filename Name of the file containing the pairs.
     */
    void init(const std::string& filename);

    /**
     *  Process the query and sets the response to that query. @ref Record
     *  @param query @ref Query that will be processed.
     *  @param response @ref Response that will be answered.
     */
    void process(const Query& query, Response& response) noexcept;

protected:
    /**
     *  Extracts the ipAddress-hostname pair from a string line and adds it to
     *  the list of records.
     *  @param line Line read from the hosts file containing the ipAddress-hostname info
     */
    void store(const std::string& line) noexcept;

    /**
     *  Structure to hold the ipAddress-hostname pairs.
     */
    struct Record {
        std::string ipAddress;  // IP address in dot notation
        std::string domainName;

        Record() = default;
        Record(std::string ip, std::string domain) :
            ipAddress(std::move(ip)), domainName(std::move(domain)) { }
    };

    /**
     *  Prints all records from the list.
     */
    void print_records() noexcept;

    /**
     *  Convert IN-ADDR.ARPA domain to an IP address in dot notation
     *  @param domain The domain name
     *  @return The IP addrress formatted in dot notation.
     */
    std::string convert(const std::string& domain) noexcept;

    /**
     *  Finds in the list the domanin corresponding to the ipAddress
     *  @param ipAddress IP addrress in dot notation
     *  @return The domain name found. An empty string if no domain was found.
     */
    std::string find(const std::string& ipAddress) noexcept;

    /**
     *  Pointer to the start of the list of records.
     */
    std::list<Record> m_record_list;
};

} // namespace dns
