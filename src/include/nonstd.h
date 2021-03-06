#pragma once

#include <chrono>
#include <iterator>
#include <utility>

namespace nonstd {

using std::chrono::milliseconds;
using std::chrono::seconds;

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

} // namespace nonstd
