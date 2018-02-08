#pragma once

//
// template<class T>
// class future {
// public:
//     using promise_type = promise<T>;
//
//     future() noexcept = default;
//     future(const future&) = delete;
//     future(future&&) noexcept = default;
//     future& operator=(future&&) noexcept = default;
//     future& operator=(const future&) = delete;
//
//     void swap(future<T>& rhs);
//     bool valid() const;
//     bool ready() const;
//     void wait();
//     template<class Clock, class Duration> std::cv_status wait_until(const std::chrono::time_point<Clock, Duration>& deadline);
//     T get();
//     std::exception_ptr get_exception();
//     void then_set(promise<T> p);
//     template<class F> auto finally(F func_taking_void_and_returning_void) -> future<T>;
//     template<class F> auto then(F func_taking_future_and_returning_R) -> future<R>;
//     template<class F> auto then_f(F func_taking_future_and_returning_FR) -> FR;
//     template<class F> auto on_value(F func_taking_value_and_returning_R) -> future<R>;
//     template<class F> auto on_exception(F func_taking_exception_ptr_and_returning_R) -> future<R>;
//     template<class F> auto on_value_f(F func_taking_value_and_returning_FR) -> FR;
//     template<class F> auto on_exception_f(F func_taking_exception_ptr_and_returning_FR) -> FR;
// };
//

#include "nonstd.h"
#include "nonstd-function.h"
#include "nonstd-optional.h"
#include "nonstd-regular-void.h"

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
    void emplace_value(Args&&... args) {
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

    template<class F>
    void set_value_from_result_of(F&& f) {
        assert(m_state == UNREADY);
        REGULAR_INVOKE(m_value.emplace, std::forward<F>(f)());
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

    std::exception_ptr get_exception_assuming_ready() {
        return std::move(m_exception);
    }
};

template<class T> class future;

template<class T>
class promise {
    std::shared_ptr<future_shared_state<T>> m_ptr;

    promise(std::shared_ptr<future_shared_state<T>> p) : m_ptr(std::move(p)) {}
    void detach() { m_ptr = nullptr; }

    template<class U> friend class future;
public:
    using future_type = future<T>;

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

    bool valid() const noexcept { return m_ptr != nullptr; }
    bool ready() const noexcept { return valid() && m_ptr->ready(); }

    template<class F>
    void set_value_from_result_of(F&& f) {
        assert(valid());
        m_ptr->set_value_from_result_of(std::forward<F>(f));
    }

    template<bool B = !std::is_void<T>::value, class = std::enable_if_t<B>>
    void set_value(std::conditional_t<B, T, int> t) {
        assert(valid());
        m_ptr->set_value_from_result_of([&]() -> decltype(auto) { return std::move(t); });
    }

    template<bool B = std::is_void<T>::value, class = std::enable_if_t<B>>
    void set_value() {
        assert(valid());
        m_ptr->set_value_from_result_of([&](){});
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
    using promise_type = promise<T>;

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

    std::exception_ptr get_exception() {
        wait();
        auto sptr = std::move(m_ptr);
        return std::move(sptr->get_exception_assuming_ready());
    }

    void then_set(promise<T> p) {
        auto input = std::move(m_ptr);
        auto output = std::move(p.m_ptr);
        input->set_on_value([input, output]() {
            REGULAR_INVOKE(output->emplace_value, input->get_value_assuming_ready());
        });
        input->set_on_exception([input, output]() {
            output->set_exception(input->get_exception_assuming_ready());
        });
    }

    template<class F>
    auto then(F func) {
        assert(valid());
        auto input = std::move(m_ptr);
        using R = decltype(func(future(input)));
        promise<R> p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value_or_exception([input, output, func = std::move(func)]() mutable {
            try {
                REGULAR_INVOKE(output->emplace_value, func(future(input)));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        return result;
    }

    template<class F>
    auto then_f(F func_returning_future) {
        assert(valid());
        auto input = std::move(m_ptr);
        using PromiseOfR = typename decltype(func_returning_future(future(input)))::promise_type;
        PromiseOfR p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value_or_exception([input, output, func_returning_future = std::move(func_returning_future)]() mutable {
            try {
                func_returning_future(future(input)).then_set(PromiseOfR(output));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        return result;
    }

    template<class F>
    auto finally(F func_returning_void) {
        static_assert(std::is_void<decltype(func_returning_void())>::value, "");
        return this->then_f([func_returning_void = std::move(func_returning_void)](auto f) {
            func_returning_void();
            return f;
        });
    }

    template<class F>
    auto on_value(F func_taking_value) {
        assert(valid());
        auto input = std::move(m_ptr);
        using R = decltype(UNEVALUATED_INVOKE(func_taking_value, input->get_value_assuming_ready()));
        promise<R> p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value([input, output, func_taking_value = std::move(func_taking_value)]() mutable {
            try {
                REGULAR_INVOKE(output->emplace_value, REGULAR_INVOKE(func_taking_value, input->get_value_assuming_ready()));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        input->set_on_exception([input, output]() mutable {
            output->set_exception(input->get_exception_assuming_ready());
        });
        return result;
    }

    template<class F>
    auto on_exception(F func_taking_exception_ptr) {
        assert(valid());
        auto input = std::move(m_ptr);
        using R = decltype(UNEVALUATED_INVOKE(func_taking_exception_ptr, input->get_value_assuming_ready()));
        promise<R> p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value([input, output]() {
            REGULAR_INVOKE(output->emplace_value, input->get_value_assuming_ready());
        });
        input->set_on_exception([input, output, func_taking_exception_ptr = std::move(func_taking_exception_ptr)]() mutable {
            try {
                REGULAR_INVOKE(output->emplace_value, func_taking_exception_ptr(input->get_exception_assuming_ready()));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        return result;
    }

    template<class F>
    auto on_value_f(F func_taking_value_returning_future) {
        assert(valid());
        auto input = std::move(m_ptr);
        using PromiseOfR = typename decltype(UNEVALUATED_INVOKE(func_taking_value_returning_future, input->get_value_assuming_ready()))::promise_type;
        PromiseOfR p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value([input, output, func_taking_value_returning_future = std::move(func_taking_value_returning_future)]() mutable {
            try {
                REGULAR_INVOKE(func_taking_value_returning_future, input->get_value_assuming_ready()).then_set(PromiseOfR(output));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        input->set_on_exception([input, output]() mutable {
            output->set_exception(input->get_exception_assuming_ready());
        });
        return result;
    }

    template<class F>
    auto on_exception_f(F func_taking_exception_ptr_returning_future) {
        assert(valid());
        auto input = std::move(m_ptr);
        using PromiseOfR = typename decltype(func_taking_exception_ptr_returning_future(input->get_exception_assuming_ready()))::promise_type;
        PromiseOfR p;
        auto result = p.get_future();
        auto output = std::move(p.m_ptr);
        input->set_on_value([input, output]() {
            REGULAR_INVOKE(output->emplace_value, input->get_value_assuming_ready());
        });
        input->set_on_exception([input, output, func_taking_exception_ptr_returning_future = std::move(func_taking_exception_ptr_returning_future)]() mutable {
            try {
                func_taking_exception_ptr_returning_future(input->get_exception_assuming_ready()).then_set(PromiseOfR(output));
            } catch (...) {
                output->set_exception(std::current_exception());
            }
        });
        return result;
    }
};

template<class T>
nonstd::future<T> make_ready_future(T t) {
    nonstd::promise<T> prom;
    auto fut = prom.get_future();
    prom.set_value_from_result_of([&]() { return std::move(t); });
    return fut;
}

inline nonstd::future<void> make_ready_future() {
    nonstd::promise<void> prom;
    auto fut = prom.get_future();
    prom.set_value_from_result_of([&]() {});
    return fut;
}

template<class F, class R = decltype(std::declval<F>()())>
nonstd::future<R> async(F f) {
    nonstd::promise<R> prom;
    auto fut = prom.get_future();
    std::thread t([prom = std::move(prom), f = std::move(f)]() mutable {
        try {
            prom.set_value_from_result_of(f);
        } catch (...) {
            prom.set_exception(std::current_exception());
        }
    });
    t.detach();
    return fut;
}

} // namespace nonstd
