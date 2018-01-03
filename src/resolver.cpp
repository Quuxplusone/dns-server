
#include "logger.h"
#include "resolver.h"
#include "query.h"
#include "response.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace dns;

void Resolver::init(const std::string& filename)
{
    Logger& logger = Logger::instance();
    std::string text("Resolver::init() | filename: ");
    text += filename;
    logger.trace(text);

    std::ifstream file(filename.data());

    if (!file) {
        std::string text("Could not open file: ");
        text += filename;
        logger.error(text);
        throw dns::Exception(text);
    }

    std::string line;
    while (!file.eof()) {
        std::getline(file, line);
        store(line);
    }

    file.close();

    print_records();
}

void Resolver::store(const std::string& line) noexcept
{
    std::string::size_type ipAddressEndPos = line.find_first_of(" ");
    if (ipAddressEndPos == std::string::npos) return;

    std::string::size_type domainNameStartPos = line.find_last_of(" ");
    if (domainNameStartPos == std::string::npos) return;
    domainNameStartPos += 1;

    std::string ipAddress(line, 0, ipAddressEndPos);
    std::string domainName(line, domainNameStartPos, line.length());

    try {
        m_record_list.emplace_back(ipAddress, domainName);
    } catch (const std::exception& e) {
        Logger& logger = Logger::instance();
        logger.error("Could not create Record object");
    }
}

void Resolver::print_records() noexcept
{
    std::cout << "Reading records from file..." << std::endl;
    if (m_record_list.empty()) {
        std::cout << "No records on list." << std::endl;
        return;
    }
    for (auto&& record : m_record_list) {
        std::cout << "Record: " << record.ipAddress.data();
        std::cout << " - " << record.domainName.data() << std::endl;
    }
    std::cout << std::endl;
}

std::string Resolver::find(const std::string& ipAddress) noexcept
{
    Logger& logger = Logger::instance();
    std::string text("Resolver::find() | ipAddress: ");
    text += ipAddress;

    std::string domainName;
    for (auto&& record : m_record_list) {
        if (record.ipAddress == ipAddress) {
            domainName = record.domainName;
            break;
        }
    }

    text += " ---> ";
    text += domainName;
    logger.trace(text);

    return domainName;
}

void Resolver::process(const Query& query, Response& response) noexcept
{
    Logger& logger = Logger::instance();
    std::string text("Resolver::process()");
    text += query.asString();
    logger.trace(text);

    std::string qName = query.getQName();
    std::string ipAddress = convert(qName);
    std::string domainName = find( ipAddress );

    response.setID( query.getID() );
    response.setQdCount(1);
    response.setAnCount(1);
    response.setName( query.getQName() );
    response.setType( query.getQType() );
    response.setClass( query.getQClass() );
    response.setRdata(domainName);

    std::cout << std::endl << "Query for: " << ipAddress;
    std::cout << std::endl << "Response with: ";

    if (domainName.empty()) {
        std::cout << "NameError" << std::endl;
        response.setRCode(Response::NameError);
        response.setRdLength(1); // null label
    } else {
        std::cout << domainName << std::endl;
        response.setRCode(Response::Ok);
        response.setRdLength(domainName.size()+2); // + initial label length & null label
    }

    text = "Resolver::process()";
    text += response.asString();
    logger.trace(text);
}

std::string Resolver::convert(const std::string& qName) noexcept
{
    int pos = qName.find(".in-addr.arpa");
    if (pos == std::string::npos) return std::string();

    std::string tmp(qName, 0, pos);
    std::string ipAddress;
    while ((pos = tmp.rfind('.')) != std::string::npos) {
        ipAddress.append(tmp, pos+1, tmp.size());
        ipAddress.append(".");
        tmp.erase(pos, tmp.size());
    }
    ipAddress.append(tmp, 0, tmp.size());

    return ipAddress;
}
