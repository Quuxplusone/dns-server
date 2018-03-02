
#include "bus.h"
#include "exception.h"
#include "message.h"
#include "nonstd.h"
#include "question.h"
#include "stub-resolver.h"

#include <assert.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace dns;

StubResolver::StubResolver(bus::Bus& bus, Upstream upstream) : m_bus(bus)
{
    m_upstreams.push_back(std::move(upstream));
}

static Message recv_message_from_socket(int sockfd)
{
    char buffer[1024];
    Message response;

    if (true) {
        struct sockaddr_in serverAddress;
        socklen_t addrLen = sizeof serverAddress;
        int nbytes = recvfrom(
            sockfd,
            buffer, sizeof buffer,
            0,
            reinterpret_cast<struct sockaddr *>(&serverAddress), &addrLen
        );
        if (nbytes <= 0) {
            throw dns::Exception("recvfrom returned ", nbytes);
        }
        std::cout << "Received " << nbytes << " bytes!" << std::endl;
        const char *parsed = nullptr;
        try {
            parsed = response.decode(buffer, buffer + nbytes);
        } catch (const dns::Exception& e) {
            throw dns::Exception("During packet decode: ", e.what());
        }
        if (parsed == nullptr) {
            throw dns::Exception("Failed to parse packet of length ", nbytes);
        }
        if (parsed != buffer + nbytes) {
            std::cout << "Packet of length " << nbytes
                << " parsed as message of length " << (parsed - buffer)
                << " with some trailing bytes" << std::endl;
        }
    }

    return response;
}

static void send_message_to_socket(const Message& query, int sockfd, const Upstream& upstream)
{
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
}

static bool looks_like_attempted_response_to(const Message& response, const Message& query)
{
    if (!response.is_response()) {
        std::cout << "Wasn't a response at all" << std::endl;
    } else if (response.id() != query.id()) {
        std::cout << "Was a response with an invalid query-id" << std::endl;
    } else if (response.questions().size() != 1) {
        std::cout << "Was a response with more than one question, or no questions" << std::endl;
    } else if (response.questions().front() != query.questions().front()) {
        std::cout << "Was a response with the wrong question" << std::endl;
    } else {
        return true;
    }
    return false;
}

nonstd::future<Message> StubResolver::loop_until_recv_response_from_socket(int sockfd, Message query) const
{
    return m_bus.when_ready_to_recv(sockfd)
    .on_value_f([this, query = std::move(query), sockfd]() mutable {
        try {
            Message response = recv_message_from_socket(sockfd);
            if (looks_like_attempted_response_to(response, query)) {
                return nonstd::make_ready_future(std::move(response));
            }
        } catch (dns::Exception& e) {
            std::cout << e.what() << std::endl;
        }
        return loop_until_recv_response_from_socket(sockfd, std::move(query));
    });
}

nonstd::future<Message> StubResolver::async_resolve(const Message& query, nonstd::milliseconds timeout) const
{
    const Upstream& upstream = this->m_upstreams.at(0);

    Upstream ephemeral_port("127.0.0.1", 0);
    int sockfd = ephemeral_port.bind_udp_socket(timeout);
    assert(sockfd > 0);

    // Send the query to the upstream server.
    return m_bus.when_ready_to_send(sockfd).on_value_f([this, query, sockfd, upstream]() {
        send_message_to_socket(query, sockfd, upstream);

        // Now listen until we hear a response.
        return this->loop_until_recv_response_from_socket(sockfd, query);
    }).finally([sockfd]() {
        close(sockfd);
    });
}
