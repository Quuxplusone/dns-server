
#include "logger.h"
#include "resolver.h"
#include "query.h"
#include "response.h"

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
        store(line);
    }
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

    m_record_list.emplace_back(ipAddress, domainName);
}

void Resolver::print_records() noexcept
{
    if (m_record_list.empty()) {
        std::cout << "No records on list." << std::endl;
    } else {
        for (auto&& record : m_record_list) {
            std::cout << "Record: " << record.ipAddress.data();
            std::cout << " - " << record.domainName.data() << std::endl;
        }
    }
}

std::string Resolver::find(const std::string& ipAddress) noexcept
{
    std::string domainName;
    for (auto&& record : m_record_list) {
        if (record.ipAddress == ipAddress) {
            domainName = record.domainName;
            break;
        }
    }
    Logger::trace("Resolver::find() | ipAddress: ", ipAddress, " ---> ", domainName);
    return domainName;
}

void Resolver::process(const Query& query, Response& response) noexcept
{
    std::string qName = query.getQName().repr();
    std::string ipAddress = convert(qName);
    std::string domainName = find(ipAddress);

    response.setID(query.getID());
    response.setQdCount(1);
    response.setAnCount(1);
    response.setName(query.getQName());
    response.setType(query.getQType());
    response.setClass(query.getQClass());

    std::cout << std::endl << "Query for: " << ipAddress;
    std::cout << std::endl << "Response with: ";

    if (domainName.empty()) {
        std::cout << "NameError" << std::endl;
        response.setRCode(Response::NXDOMAIN);
        response.setRdata(Name("."));
        response.setRdLength(1); // null label
    } else {
        std::cout << domainName << std::endl;
        response.setRCode(Response::NOERROR);
        response.setRdata(Name(domainName.c_str()));
        response.setRdLength(domainName.size()+2); // + initial label length & null label
    }
}

std::string Resolver::convert(const std::string& qName) noexcept
{
    int pos = qName.find(".in-addr.arpa");
    if (pos == std::string::npos) return "";

    std::string tmp(qName, 0, pos);
    std::string ipAddress;
    while ((pos = tmp.rfind('.')) != std::string::npos) {
        ipAddress.append(tmp, pos+1, tmp.size());
        ipAddress += '.';
        tmp.erase(pos, tmp.size());
    }
    ipAddress += tmp;
    return ipAddress;
}
