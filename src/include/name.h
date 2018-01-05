#pragma once

#include <string>
#include <vector>

namespace dns {

/**
 *  Class that represents a DNS label (a possibly empty sequence of characters).
 */
class Label {
public:
    Label() = default;
    explicit Label(std::string s) : m_str(s) {}

    bool operator==(const Label& rhs) const noexcept;
    bool operator!=(const Label& rhs) const noexcept { return !(*this == rhs); }
    bool operator<(const Label& rhs) const noexcept;
    bool operator<=(const Label& rhs) const noexcept { return !(rhs < *this); }
    bool operator>(const Label& rhs) const noexcept { return (rhs < *this); }
    bool operator>=(const Label& rhs) const noexcept { return !(*this < rhs); }

    const char *data() const noexcept { return m_str.data(); }
    size_t size() const noexcept { return m_str.size(); }
    bool empty() const noexcept { return m_str.empty(); }
    void clear() noexcept { m_str.clear(); }
    void operator+=(char c) { m_str += c; }

    std::string repr() const;

    static Label asterisk() { return Label("*"); }
    bool is_asterisk() const noexcept { return m_str == "*"; }

private:
    std::string m_str;
};

/**
 *  Class that represents a DNS domain name (a possibly empty sequence of labels).
 */
class Name {
public:
    Name() = default;
    explicit Name(const char *repr);

    bool operator==(const Name& rhs) const noexcept { return m_labels == rhs.m_labels; }
    bool operator!=(const Name& rhs) const noexcept { return m_labels != rhs.m_labels; }
    const std::vector<Label>& labels() const noexcept { return m_labels; }

    char *encode(char *dst, const char *end) const noexcept;
    const char *decode(const char *src, const char *end);

    const char *decode_repr(const char *src, const char *end);
    std::string repr() const;
private:
    std::vector<Label> m_labels;
};

} // namespace dns
