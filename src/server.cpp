
#include "logger.h"
#include "server.h"
#include "resolver.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>

using namespace dns;

void Server::init(int port)
{
    Logger& logger = Logger::instance();
    logger.trace("Server::init()");

    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(port);

    int rbind = bind(m_sockfd, (struct sockaddr *) & m_address, sizeof (struct sockaddr_in));

    if (rbind != 0) {
        throw dns::Exception("Could not bind: ", strerror(errno));
    }

    std::cout << "Listening in port: " << port << ", sockfd: " << m_sockfd << std::endl;
}

void Server::run() noexcept
{
    Logger& logger = Logger::instance();
    logger.trace("Server::run()");

    std::cout << "DNS Server running..." << std::endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof (struct sockaddr_in);

    while (true) {
        int nbytes = recvfrom(m_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &clientAddress, &addrLen);

        m_query.decode(buffer, nbytes);
        m_query.asString();

        m_resolver.process(m_query, m_response);

        m_response.asString();
        memset(buffer, 0, BUFFER_SIZE);
        nbytes = m_response.code(buffer);

        sendto(m_sockfd, buffer, nbytes, 0, (struct sockaddr *) &clientAddress, addrLen);
    }
}