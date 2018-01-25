#pragma once

#include "nonstd.h"
#include "nonstd-function.h"
#include "nonstd-optional.h"

#include <atomic>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace nonstd {

template<class T>
class future_shared_state
{
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    std::atomic<int> m_state{0};
    nonstd::optional<T> m_value;
    std::exception_ptr m_exception;
    nonstd::unique_function<void()> m_on_value;
    nonstd::unique_function<void()> m_on_exception;
public:
    enum {
        UNREADY = 0,
        READY_WITH_VALUE = 1,
        READY_WITH_EXCEPTION = 2
    };
    bool ready() const { return m_state == READY_WITH_VALUE || m_state == READY_WITH_EXCEPTION; }

    template<class... Args>
    void set_value(Args&&... args) {
        assert(m_state == UNREADY);
        m_value.emplace(std::forward<Args>(args)...);
        unique_function<void()> continuation;
        if (true) {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_state = READY_WITH_VALUE;
            m_on_value.swap(continuation);
            m_cv.notify_all();
        }
        if (continuation) {
            continuation();
        }
    }

    void set_exception(std::exception_ptr ex) {
        assert(m_state == UNREADY);
        m_exception = std::move(ex);
        unique_function<void()> continuation;
        if (true) {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_state = READY_WITH_EXCEPTION;
            m_on_exception.swap(continuation);
            m_cv.notify_all();
        }
        if (continuation) {
            continuation();
        }
    }

    void set_on_value(unique_function<void()> continuation) {
        bool just_run_it = false;
        if (true) {
            std::lock_guard<std::mutex> lk(m_mtx);
            just_run_it = (m_state == READY_WITH_VALUE);
            if (!just_run_it) {
                m_on_value.swap(continuation);
            }
        }
        if (just_run_it) {
            continuation();
        }
    }

    void set_on_exception(unique_function<void()> continuation) {
        bool just_run_it = false;
        if (true) {
            std::lock_guard<std::mutex> lk(m_mtx);
            just_run_it = (m_state == READY_WITH_EXCEPTION);
            if (!just_run_it) {
                m_on_exception.swap(continuation);
            }
        }
        if (just_run_it) {
            continuation();
        }
    }

    void set_on_value_or_exception(unique_function<void()> continuation) {
        bool just_run_it = false;
        if (true) {
            std::lock_guard<std::mutex> lk(m_mtx);
            just_run_it = (m_state == READY_WITH_VALUE || m_state == READY_WITH_EXCEPTION);
            if (!just_run_it) {
                m_on_value.swap(continuation);
                m_on_exception = std::ref(m_on_value);
            }
        }
        if (just_run_it) {
            continuation();
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lk(m_mtx);
        while (m_state == UNREADY) {
            m_cv.wait(lk);
        }
    }

    template<class Clock, class Duration>
    std::cv_status wait_until(const std::chrono::time_point<Clock, Duration>& deadline) {
        std::unique_lock<std::mutex> lk(m_mtx);
        bool satisfied = m_cv.wait_until(lk, deadline, [&](){ return ready(); });
        return satisfied ? std::cv_status::no_timeout : std::cv_status::timeout;
    }

    decltype(auto) get_value_assuming_ready() {
        if (m_value) {
            return *m_value;
        } else {
            std::rethrow_exception(m_exception);
        }
    }
};

template<class T> class future;
template<class T> class shared_future;

template<class T>
class promise {
    std::shared_ptr<future_shared_state<T>> m_ptr;

public:
    promise() { m_ptr = std::make_shared<future_shared_state<T>>(); }

    promise(const promise&) = delete;
    promise(promise&&) noexcept = default;
    promise& operator=(promise&& rhs) noexcept { promise(std::move(rhs)).swap(*this); return *this; }
    promise& operator=(const promise&) = delete;

    void swap(promise& rhs) { m_ptr.swap(rhs.m_ptr); }

    ~promise() {
        if (valid() && !ready()) {
            set_exception(nonstd::make_exception_ptr("oops"));
        }
    }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    template<bool B = not std::is_void<T>::value, class = std::enable_if_t<B>>
    void set_value(std::conditional_t<B, T, int> t) {
        assert(valid());
        m_ptr->set_value(std::move(t));
    }

    template<bool B = std::is_void<T>::value, class = std::enable_if_t<B>>
    void set_value() {
        assert(valid());
        m_ptr->set_value();
    }

    void set_exception(std::exception_ptr ex) {
        assert(valid());
        m_ptr->set_exception(std::move(ex));
    }

    future<T> get_future() {
        assert(valid());
        return future<T>(m_ptr);
    }
};

template<class T>
class future {
    std::shared_ptr<future_shared_state<T>> m_ptr;

    future(std::shared_ptr<future_shared_state<T>> p) : m_ptr(std::move(p)) {}

    friend class promise<T>;
public:
    future() noexcept = default;
    future(const future&) = delete;
    future(future&&) noexcept = default;
    future& operator=(future&&) noexcept = default;
    future& operator=(const future&) = delete;

    void swap(future& rhs) { m_ptr.swap(rhs.m_ptr); }

    bool valid() const { return m_ptr != nullptr; }
    bool ready() const { return valid() && m_ptr->ready(); }

    void wait() {
        assert(valid());
        m_ptr->wait();
    }

    template<class Clock, class Duration>
    std::cv_status wait_until(const std::chrono::time_point<Clock, Duration>& deadline) {
        assert(valid());
        return m_ptr->wait_until(deadline);
    }

    T get() {
        wait();
        auto sptr = std::move(m_ptr);
        return std::move(sptr->get_value_assuming_ready());
    }

    template<class F>
    auto then(F func) {
        assert(valid());
        using R = decltype(func(std::move(*this)));
        promise<R> p;
        future<R> result = p.get_future();
        unique_function<void()> continuation = [p = std::move(p), func = std::move(func), sptr = m_ptr]() mutable {
            try {
                future self(std::move(sptr));
                p.set_value(func(std::move(self)));
            } catch (...) {
                p.set_exception(std::current_exception());
            }
        };
        auto sptr = std::move(m_ptr);
        sptr->set_on_value_or_exception(std::move(continuation));
        return result;
    }

    template<class F>
    auto then_f(F func) {
        return this->then([func = std::move(func)](auto self) mutable {
            return func(std::move(self)).get();
        });
    }

    template<class F>
    auto next(F func) {
        return this->then([func = std::move(func)](auto self) {
            return func(self.get());
        });
    }

};

template<class F, class R = decltype(std::declval<F>()()), std::enable_if_t<std::is_void<R>::value, bool> = true>
nonstd::future<R> async(F f) {
    nonstd::promise<R> prom;
    nonstd::future<R> fut = prom.get_future();
    std::thread t([prom = std::move(prom), f = std::move(f)]() mutable {
        try {
            f();
            prom.set_value();
        } catch (...) {
            prom.set_exception(std::current_exception());
        }
    });
    t.detach();
    return fut;
}

template<class F, class R = decltype(std::declval<F>()()), std::enable_if_t<!std::is_void<R>::value, bool> = true>
nonstd::future<R> async(F f) {
    nonstd::promise<R> prom;
    nonstd::future<R> fut = prom.get_future();
    std::thread t([prom = std::move(prom), f = std::move(f)]() mutable {
        try {
            prom.set_value(f());
        } catch (...) {
            prom.set_exception(std::current_exception());
        }
    });
    t.detach();
    return fut;
}

} // namespace nonstd
