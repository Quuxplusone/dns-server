#pragma once

#include "message.h"
#include "question.h"

#include <functional>
#include <future>
#include <utility>

namespace dns {

/**
 *  Digger class is a socket client that makes a single query and (synchronously)
 *  waits for the response.
 */
class Digger {
    using Task = std::function<void()>;

public:
    template<class TaskScheduler>
    explicit Digger(TaskScheduler& scheduler) {
        m_schedule_task = [&](Task task) {
            scheduler(std::move(task));
        };
    }

    std::future<Message> dig(Question question) const;

private:
    std::function<void(Task)> m_schedule_task;
};

} // namespace dns
