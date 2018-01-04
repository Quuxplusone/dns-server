#pragma once

#include <exception>
#include <string>

namespace dns {

/**
 *  Exception class extends standard exception functionality and adds it the text
 *  message to inform about the reason of the exception thrown.
 */
class Exception : public std::exception {
public:
    template<class... Args>
    explicit Exception(Args&&... args) {
        int a[] = {
            [&]() { m_text += std::forward<Args>(args); return 0; }() ...
        };
        (void)a;
    }

    const char *what() const noexcept override {
        return m_text.data();
    }

private:
    std::string m_text;
};

class UnsupportedException : public Exception {
public:
    template<class... Args>
    explicit UnsupportedException(Args&&... args) : Exception(std::forward<Args>(args)...) {}
};

} // namespace dns
