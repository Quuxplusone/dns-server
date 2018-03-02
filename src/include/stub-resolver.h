#pragma once

#include "bus.h"
#include "message.h"
#include "nonstd.h"
#include "nonstd-future.h"
#include "question.h"
#include "upstream.h"

#include <vector>

namespace dns {

/**
 *  StubResolver is a socket client that makes a single query and (synchronously)
 *  waits for the response.
 */
class StubResolver {
public:
    explicit StubResolver(bus::Bus& bus, Upstream upstream);

    nonstd::future<Message> async_resolve(const Message& query, nonstd::milliseconds timeout) const;

private:
    nonstd::future<Message> loop_until_recv_response_from_socket(int sockfd, Message query) const;

    bus::Bus& m_bus;
    std::vector<Upstream> m_upstreams;
};

} // namespace dns
