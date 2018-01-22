#pragma once

#include "message.h"
#include "nonstd.h"
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
    explicit StubResolver(Upstream upstream);

    std::future<Message> async_resolve(const Message& query, nonstd::milliseconds timeout) const;

private:
    std::vector<Upstream> m_upstreams;
};

} // namespace dns
