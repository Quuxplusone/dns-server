#pragma once

#include "nonstd-future.h"

#include <memory>

namespace bus {

class BusImpl;

class Bus {
public:
    Bus();
    nonstd::future<void> when_ready_to_recv(int fd) const;
    nonstd::future<void> when_ready_to_send(int fd) const;
    ~Bus();
private:
    std::unique_ptr<BusImpl> m_impl;
};

} // namespace bus
