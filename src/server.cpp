
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

    struct sockaddr_in address {};
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
            const char *parsed = nullptr;
            try {
                parsed = query.decode(buffer, buffer + nbytes);
            } catch (const dns::Exception& e) {
                std::cout << "During packet decode: " << e.what() << std::endl;
            }
            if (parsed == nullptr) {
                std::cout << "Failed to parse packet of length " << nbytes << std::endl;
                // and blackhole the malformed packet
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
                // and blackhole the query: oops!
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
                // and blackhole the malformed packet
            } else if (query.opcode() != Opcode::QUERY) {
                std::cout << "Query had opcode " << query.opcode().repr() << ", not QUERY" << std::endl;
                Message response = query.beginResponse();
                response.setRCode(RCode::NOTIMP);
                write_out(response);
            } else if (query.questions().size() != 1) {
                std::cout << "Query contained " << (query.questions().empty() ? "no" : "multiple") << " questions in question section" << std::endl;
                Message response = Message::beginResponseTo(query);
                response.setAA(true).setRA(false).setRCode(RCode::FORMERR);
                write_out(response);
            } else if (query.answers().size() != 0) {
                std::cout << "Query contained RRs in its answer section" << std::endl;
                Message response = Message::beginResponseTo(query);
                response.setAA(true).setRA(false).setRCode(RCode::FORMERR);
                write_out(response);
            } else if (query.authority().size() != 0) {
                std::cout << "Query contained RRs in its authority section" << std::endl;
                Message response = Message::beginResponseTo(query);
                response.setAA(true).setRA(false).setRCode(RCode::FORMERR);
                write_out(response);
            } else if (query.additional().size() != 0) {
                // RFC 6891, section 7: if EDNS is unsupported, respond with FORMERR
                std::cout << "Query contained RRs in its additional section (perhaps due to EDNS?)" << std::endl;
                Message response = Message::beginResponseTo(query);
                response.setAA(true).setRA(false).setRCode(RCode::FORMERR);
                write_out(response);
            } else {
                const Question& q = query.questions().front();
                Message response = query.beginResponse();
                m_resolver.populate_response(q, response);
                write_out(response);
            }
        }
    }
}
