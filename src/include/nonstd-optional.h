#pragma once

#include <experimental/optional>

namespace nonstd {

template<class T>
class optional : public std::experimental::optional<T> {};

template<>
class optional<void> {
public:
    void emplace() { yes = true; }
    void reset() { yes = false; }
    explicit operator bool() const { return yes; }
    void operator*() const {}
private:
    bool yes = false;
};

} // namespace nonstd
