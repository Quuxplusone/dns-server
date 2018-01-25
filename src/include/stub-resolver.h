#pragma once

#include "bus.h"
#include "message.h"
#include "nonstd.h"
#include "nonstd-future.h"
#include "question.h"
#include "upstream.h"

#include <future>
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
    bus::Bus& m_bus;
    std::vector<Upstream> m_upstreams;
};

} // namespace dns
