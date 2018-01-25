
#include "bus.h"
#include "exception.h"
#include "message.h"
#include "nonstd-future.h"
#include "upstream.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <list>
#include <mutex>
#include <stdio.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace bus {

class BusImpl {
    int m_readable_masterfd;
    int m_writable_masterfd;
    std::atomic<bool> m_shutting_down;
    std::vector<std::pair<int, nonstd::promise<void>>> m_readfds;
    std::vector<std::pair<int, nonstd::promise<void>>> m_writefds;
    mutable std::list<nonstd::unique_function<bool()>> m_bookkeeping_tasks;
    mutable std::mutex m_bookkeeping_mutex;
    std::thread m_thread;

    void perform_reading(int fd)
    {
        auto it = std::find_if(m_readfds.begin(), m_readfds.end(), [fd](auto&& fd_p) {
            return (fd_p.first == fd);
        });
        assert(it != m_readfds.end());
        it->second.set_value();
        if (it + 1 != m_readfds.end()) {
            *it = std::move(m_readfds.back());
        }
        m_readfds.pop_back();
    }

    void perform_writing(int fd)
    {
        auto it = std::find_if(m_writefds.begin(), m_writefds.end(), [fd](auto&& fd_p) {
            return (fd_p.first == fd);
        });
        assert(it != m_writefds.end());
        it->second.set_value();
        if (it + 1 != m_writefds.end()) {
            *it = std::move(m_writefds.back());
        }
        m_writefds.pop_back();
    }

    bool step(nonstd::milliseconds timeout)
    {
        fd_set readfds, writefds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(m_readable_masterfd, &readfds);
        int maxfd = m_readable_masterfd;
        for (auto&& fd_p : m_readfds) { int fd = fd_p.first; FD_SET(fd, &readfds); maxfd = std::max(maxfd, fd); }
        for (auto&& fd_p : m_writefds) { int fd = fd_p.first; FD_SET(fd, &writefds); maxfd = std::max(maxfd, fd); }
        struct timeval tv = nonstd::as_timeval(timeout);
        int nready = select(maxfd + 1, &readfds, &writefds, nullptr, &tv);
        if (nready < 0) {
            throw dns::Exception("select returned ", nready, " with errno ", int(errno));
        } else if (nready == 0) {
            // The deadline was reached.
        } else {
            for (int fd = 0; fd <= maxfd; ++fd) {
                if (fd == m_readable_masterfd) {
                    // Delay this one. Let's process any real input we can, first.
                } else if (FD_ISSET(fd, &readfds)) {
                    // fd is ready for reading!
                    perform_reading(fd);
                } else if (FD_ISSET(fd, &writefds)) {
                    // fd is ready for writing!
                    perform_writing(fd);
                }
            }
            if (FD_ISSET(m_readable_masterfd, &readfds)) {
                // Someone has woken us up to do some bookkeeping task.
                char cmd;
                ssize_t nbytes = read(m_readable_masterfd, &cmd, 1);
                if (nbytes != 1) {
                    throw dns::Exception("read returned ", nbytes, " with errno ", int(errno));
                }
                nonstd::unique_function<bool()> bookkeeping;
                std::unique_lock<std::mutex> lk(m_bookkeeping_mutex);
                if (!m_bookkeeping_tasks.empty()) {
                    bookkeeping.swap(m_bookkeeping_tasks.front());
                    m_bookkeeping_tasks.pop_front();
                    lk.unlock();
                    bool stop = bookkeeping();  // perform the task, not under any mutex
                    return stop;
                }
            }
        }
        return false;  // don't stop
    }

    void set_up_masterfd()
    {
        int fds[2];
        int rc = pipe(fds);
        if (rc != 0) {
            throw dns::Exception("pipe returned ", rc, " with errno ", int(errno));
        }
        m_writable_masterfd = fds[1];
        m_readable_masterfd = fds[0];
    }

    template<class F>
    void schedule_bookkeeping_task(F callback)
    {
        if (true) {
            std::unique_lock<std::mutex> lk(m_bookkeeping_mutex);
            m_bookkeeping_tasks.emplace_back(std::move(callback));
        }
        char cmd = 'A';  // the value doesn't matter
        ssize_t rc = 0;
        while (rc == 0) { rc = write(m_writable_masterfd, &cmd, 1); }
        if (rc < 0) {
            throw dns::Exception("write returned ", rc, " with errno ", int(errno));
        }
    }

    void spawn_looping_thread()
    {
        m_thread = std::thread([this]() {
            while (true) {
                fprintf(stderr, ".\n");
                bool stop = step(nonstd::seconds(1));
                if (stop) break;
            }
        });
    }

public:
    BusImpl() : m_shutting_down(false)
    {
        set_up_masterfd();
        spawn_looping_thread();
    }

    nonstd::future<void> when_ready_to_recv(int fd) {
        nonstd::promise<void> p;
        auto f = p.get_future();
        schedule_bookkeeping_task([this, fd, p = std::move(p)]() mutable {
            m_readfds.emplace_back(fd, std::move(p));
            return false;  // don't stop
        });
        return f;
    }

    nonstd::future<void> when_ready_to_send(int fd) {
        nonstd::promise<void> p;
        auto f = p.get_future();
        schedule_bookkeeping_task([this, fd, p = std::move(p)]() mutable {
            m_writefds.emplace_back(fd, std::move(p));
            return false;  // don't stop
        });
        return f;
    }

    ~BusImpl()
    {
        nonstd::promise<void> p;
        nonstd::future<void> f = p.get_future();
        schedule_bookkeeping_task([p = std::move(p)]() mutable {
            p.set_value();
            return true;  // do stop
        });
        f.wait();
        // Now `step` will be returning `true`, causing `m_thread` to drop out of its loop and finish up.
        m_thread.join();
        close(m_readable_masterfd);
        close(m_writable_masterfd);
        // We actually expect all these vectors to be empty, I think.
        for (auto&& fd_p : m_readfds) close(fd_p.first);
        for (auto&& fd_p : m_writefds) close(fd_p.first);
    }
};

Bus::Bus()
{
    m_impl = std::make_unique<BusImpl>();
}

nonstd::future<void> Bus::when_ready_to_recv(int fd) const
{
    return m_impl->when_ready_to_recv(fd);
}

nonstd::future<void> Bus::when_ready_to_send(int fd) const
{
    return m_impl->when_ready_to_send(fd);
}

Bus::~Bus() {}

} // namespace bus
