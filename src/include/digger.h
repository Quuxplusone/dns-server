#pragma once

#include "message.h"
#include "question.h"
#include "upstream.h"

#include <future>

namespace dns {

/**
 *  Digger class is a socket client that makes a single query and (synchronously)
 *  waits for the response.
 */
class Digger {
public:
    explicit Digger() = default;

    std::future<Message> dig(Question question, Upstream upstream) const;
};

} // namespace dns
