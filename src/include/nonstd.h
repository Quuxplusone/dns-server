#pragma once

#include <chrono>
#include <exception>
#include <iterator>
#include <string>
#include <sys/time.h>
#include <utility>

namespace nonstd {

template<int I> struct priority_tag : priority_tag<I-1> {};
template<> struct priority_tag<0> {};

using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;
using time_point = std::chrono::time_point<std::chrono::steady_clock>;

inline struct timeval as_timeval(nonstd::milliseconds ms)
{
    struct timeval tv{};
    tv.tv_sec = ms.count() / 1000;
    tv.tv_usec = (ms.count() % 1000) * 1000;
    return tv;
}

template<class Ex>
std::exception_ptr make_exception_ptr(Ex&& ex)
{
    try {
        throw std::forward<Ex>(ex);
    } catch (...) {
        return std::current_exception();
    }
}

template<class Container>
class drop_container {
public:
    using iterator = decltype(std::declval<Container>().begin());

    drop_container(int i, Container ctr) : m_ctr(std::forward<Container>(ctr)), m_i(i) {}
    iterator begin() const noexcept { return std::next(m_ctr.begin(), m_i); }
    iterator end() const noexcept { return m_ctr.end(); }
private:
    Container m_ctr;
    int m_i;
};

template<class Container>
drop_container<Container&&> drop(int i, Container&& container) noexcept
{
    return drop_container<Container&&>(i, std::forward<Container>(container));
}

template<class Container>
class reversed_container {
public:
    using iterator = std::reverse_iterator<decltype(std::declval<Container>().begin())>;

    reversed_container(Container ctr) : m_ctr(std::forward<Container>(ctr)) {}
    iterator begin() const noexcept { return iterator(m_ctr.end()); }
    iterator end() const noexcept { return iterator(m_ctr.begin()); }
private:
    Container m_ctr;
};

template<class Container>
reversed_container<Container&&> reversed(Container&& container) noexcept
{
    return reversed_container<Container&&>(std::forward<Container>(container));
}


template<class T>
auto to_string_impl(T&&, priority_tag<3>) -> std::enable_if_t<
    std::is_same<T, signed char>::value ||
    std::is_same<T, unsigned char>::value,
    std::string>
{
    static_assert(sizeof(T) == 0, "nonstd::to_string cannot be used on signed/unsigned character types");
}

template<class T>
auto to_string_impl(T&& t, priority_tag<2>) -> decltype(std::string{} + std::forward<T>(t))
{
    return std::string{} + std::forward<T>(t);
}

template<class T>
auto to_string_impl(T&& t, priority_tag<1>) -> decltype(std::to_string(std::forward<T>(t)))
{
    return std::to_string(std::forward<T>(t));
}

template<class T>
std::string to_string_impl(T&&, priority_tag<0>)
{
    static_assert(sizeof(T) == 0, "nonstd::to_string cannot be used on this type");
}

template<class T>
std::string to_string(T&& t)
{
    return to_string_impl(std::forward<T>(t), priority_tag<3>());
}

} // namespace nonstd
