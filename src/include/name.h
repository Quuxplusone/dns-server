#pragma once

#include <string>
#include <vector>

namespace dns {

/**
 *  Class that represents a DNS domain name (a possibly empty sequence of labels).
 */
class Name {
public:
    Name() = default;
    explicit Name(const char *repr);

    char *encode(char *dst, const char *end) noexcept;
    const char *decode(const char *src, const char *end);

    std::string repr() const;
    const char *decode_repr(const char *src, const char *end);
private:
    std::vector<std::string> m_labels;
};

} // namespace dns
