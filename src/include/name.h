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

    bool operator==(const Name& rhs) const noexcept { return m_labels == rhs.m_labels; }
    bool operator!=(const Name& rhs) const noexcept { return m_labels != rhs.m_labels; }

    char *encode(char *dst, const char *end) const noexcept;
    const char *decode(const char *src, const char *end);

    const char *decode_repr(const char *src, const char *end);
    std::string repr() const;
private:
    std::vector<std::string> m_labels;
};

} // namespace dns
