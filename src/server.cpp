
#include "exception.h"
#include "message.h"
#include "question.h"
#include "resolver.h"
#include "server.h"

#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

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
    std::cout << "DNS Server running..." << std::endl;

    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof clientAddress;

    while (true) {
        Message query;

        auto read_in = [&]() -> bool {
            char buffer[1024];
            int nbytes = recvfrom(
                m_sockfd,
                buffer, sizeof buffer,
                0,
                reinterpret_cast<struct sockaddr *>(&clientAddress), &addrLen
            );
            const char *parsed = query.decode(buffer, buffer + nbytes);
            if (parsed == nullptr) {
                std::cout << "Failed to parse packet of length " << nbytes << std::endl;
                return false;
            }
            if (parsed != buffer + nbytes) {
                std::cout << "Packet of length " << nbytes
                    << " parsed as message of length " << (parsed - buffer)
                    << " with some trailing bytes" << std::endl;
            }
            return true;
        };
        auto write_out = [&](const Message& response) {
            char buffer[512];
            const char *written = response.encode(buffer, buffer + sizeof buffer);
            if (written == nullptr) {
                std::cout << "Buffer wasn't long enough to encode response packet" << std::endl;
            } else {
                sendto(
                    m_sockfd,
                    buffer, (written - buffer),
                    0,
                    reinterpret_cast<struct sockaddr *>(&clientAddress), addrLen
                );
            }
        };
        if (read_in()) {
            if (query.is_response()) {
                std::cout << "Packet was an unsolicited response, not a query" << std::endl;
            } else if (query.questions().size() == 0) {
                std::cout << "Query contained no questions in question section" << std::endl;
            } else if (query.questions().size() >= 2) {
                std::cout << "Query contained multiple questions in question section" << std::endl;
            } else {
                const Question& q = query.questions().front();
                Message response;
                response.setInResponseTo(query);
                m_resolver.populate_response(q, response);
                write_out(response);
            }
        }
    }
}
