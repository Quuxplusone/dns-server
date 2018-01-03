
#include "logger.h"
#include "server.h"
#include "resolver.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>

using namespace dns;

void Server::bind_to(int port)
{
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int rbind = bind(m_sockfd, reinterpret_cast<struct sockaddr *>(&address), sizeof address);
    if (rbind != 0) {
        throw dns::Exception("Could not bind: ", strerror(errno));
    }
}

void Server::run() noexcept
{
    Logger::trace("Server::run()");

    std::cout << "DNS Server running..." << std::endl;

    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof clientAddress;

    while (true) {
        int nbytes = recvfrom(
            m_sockfd,
            buffer, sizeof buffer,
            0,
            reinterpret_cast<struct sockaddr *>(&clientAddress), &addrLen
        );

        m_query.decode(buffer, nbytes);

        m_resolver.process(m_query, m_response);

        nbytes = m_response.code(buffer);

        sendto(
            m_sockfd,
            buffer, nbytes,
            0,
            reinterpret_cast<struct sockaddr *>(&clientAddress), addrLen
        );
    }
}
