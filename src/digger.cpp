
#include "digger.h"
#include "exception.h"
#include "message.h"
#include "question.h"

#include <arpa/inet.h>
#include <future>
#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace dns;

std::future<Message> Digger::dig(Question question, Upstream upstream) const
{
    Message query;
    query.setID(rand());
    query.setOpcode(Opcode::QUERY);
    query.setQR(false);
    query.setRD(false);
    query.add_question(std::move(question));

    Upstream ephemeral_port("127.0.0.1", 0);

    int sockfd = ephemeral_port.bind_udp_socket();

    char buffer[512];
    const char *end = query.encode(buffer, buffer + sizeof buffer);
    if (end == nullptr) {
        throw dns::Exception("Buffer wasn't long enough to encode query");
    }
    for (const char *src = buffer; src != end; ) {
        int sent = sendto(
            sockfd,
            src, (end - src),
            0,
            upstream.sockaddr(), upstream.sockaddr_length()
        );
        if (sent < 0) {
            throw dns::Exception("sendto() failed: ", strerror(errno));
        }
        std::cout << "Sent " << sent << " bytes!" << std::endl;
        src += sent;
    }

    // Now listen until we hear a response. (TODO: this is awkward and bad)

    Message response;

    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t addrLen = sizeof clientAddress;
        int nbytes = recvfrom(
            sockfd,
            buffer, sizeof buffer,
            0,
            reinterpret_cast<struct sockaddr *>(&clientAddress), &addrLen
        );
        std::cout << "Received " << nbytes << " bytes!" << std::endl;
        if (nbytes <= 0) {
            continue;
        }
        const char *parsed = nullptr;
        try {
            parsed = response.decode(buffer, buffer + nbytes);
        } catch (const dns::Exception& e) {
            std::cout << "During packet decode: " << e.what() << std::endl;
            continue;
        }
        if (parsed == nullptr) {
            std::cout << "Failed to parse packet of length " << nbytes << std::endl;
            continue;
        }
        if (parsed != buffer + nbytes) {
            std::cout << "Packet of length " << nbytes
                << " parsed as message of length " << (parsed - buffer)
                << " with some trailing bytes" << std::endl;
        }
        break;
    }

    close(sockfd);

    std::promise<Message> p;
    p.set_value(std::move(response));
    return p.get_future();
}
