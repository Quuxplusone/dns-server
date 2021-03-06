
#include "exception.h"
#include "name.h"
#include "symboltable.h"

#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <string>
#include <string.h>

using namespace dns;

bool Label::operator==(const Label& rhs) const noexcept
{
    if (m_str.size() != rhs.m_str.size()) {
        return false;
    }
    for (int i = 0; i < m_str.size(); ++i) {
        if (toupper(m_str[i]) != toupper(rhs.m_str[i])) {
            return false;
        }
    }
    return true;
}

bool Label::operator<(const Label& rhs) const noexcept
{
    int n = std::min(m_str.size(), rhs.m_str.size());
    for (int i = 0; i < n; ++i) {
        if (toupper(m_str[i]) != toupper(rhs.m_str[i])) {
            return (toupper(m_str[i]) < toupper(rhs.m_str[i]));
        }
    }
    return n < rhs.m_str.size();
}

static bool can_appear_unquoted_in_label(char ch)
{
    switch (ch) {
        case '-': case '_': case '*': case '.': case '\\': return true;
        default: return isalnum(ch);
    }
}

Name::Name(const char *repr)
{
    const char *end = strchr(repr, '\0');
    const char *parsed = decode_repr(repr, end);
    if (parsed != end) {
        throw dns::Exception("name contained trailing unparsed characters");
    }
}

const char *Name::decode_repr(const char *repr, const char *end)
{
    bool saw_double_quotes = false;
    if (repr != end && *repr == '"') {
        saw_double_quotes = true;
        ++repr;
    }
    Label current_label;
    const char *p = repr;
    while (true) {
        if (p == end) {
            if (saw_double_quotes) {
                throw dns::Exception("quoted name ends without terminating double-quotes");
            }
            break;
        }
        if (isspace(*p) && !saw_double_quotes) {
            break;
        }
        if (*p == '.') {
            if (!m_labels.empty() && m_labels.back().empty()) {
                throw dns::Exception("name contains an empty label");
            }
            m_labels.emplace_back(std::move(current_label));
            current_label.clear();
            if (m_labels.size() > 255) {
                throw dns::Exception("name contains more than 255 labels");
            }
        } else if (*p == '\\') {
            ++p;
            if (p == end) {
                throw dns::Exception("name ends with unterminated backslash-escape");
            }
            current_label += (*p);
            if (current_label.size() > 63) {
                throw dns::Exception("name contains a label longer than 63 characters");
            }
        } else if (saw_double_quotes && *p == '"') {
            ++p;
            break;
        } else if (!saw_double_quotes && !can_appear_unquoted_in_label(*p)) {
            throw dns::Exception("unusual name requires double quotes");
        } else {
            current_label += (*p);
            if (current_label.size() > 63) {
                throw dns::Exception("name contains a label longer than 63 characters");
            }
        }
        ++p;
    }
    if (!current_label.empty()) {
        throw dns::UnsupportedException("name " + std::string(repr, p) + " without trailing dot is confusing");
    }
    if (m_labels.empty()) {
        throw dns::Exception("empty name is not allowed; did you mean \".\"?");
    }
    if (!m_labels.back().empty()) {
        m_labels.emplace_back("");
    }
    return p;
}

std::string Label::repr() const
{
    std::string result;
    for (char ch : m_str) {
        switch (ch) {
            default: result += ch; break;
            case '\\': result += "\\\\"; break;
            case '.': result += "\\."; break;
            case '"': result += "\\\""; break;
        }
    }
    return result;
}

std::string Name::repr() const
{
    std::string result;
    for (auto&& label : m_labels) {
        result += label.repr();
        if (!label.empty()) result += '.';
    }
    bool require_double_quotes = false;
    for (char ch : result) {
        if (!can_appear_unquoted_in_label(ch)) {
            require_double_quotes = true;
            break;
        }
    }
    if (require_double_quotes) {
        return '"' + result + '"';
    } else {
        return result;
    }
}

char *Name::encode(char *dst, const char *end) const noexcept
{
    if (dst == nullptr) return nullptr;
    for (auto&& label : m_labels) {
        if ((end - dst) < label.size() + 1) return nullptr;
        *dst++ = label.size();
        if (label.size() != 0) {
            memcpy(dst, label.data(), label.size());
            dst += label.size();
        }
    }
    return dst;
}

const char *Name::decode(const SymbolTable& syms, const char *src, const char *end)
{
    if (src == nullptr || src == end) return nullptr;
    m_labels.clear();
    int length = -1;
    do {
        if (src == end) return nullptr;
        length = static_cast<uint8_t>(*src++);
        if ((length & 0xC0) == 0x00) {
            if (end - src < length) return nullptr;
            std::string label(length, '\0');
            memcpy(&label[0], src, length);
            m_labels.emplace_back(std::move(label));
            src += length;
        } else if ((length & 0xC0) == 0xC0) {
            auto it = syms.find(length);
            if (it == syms.end()) {
                return nullptr;
            }
            const Name& target = it->second;
            m_labels.insert(m_labels.end(), target.labels().begin(), target.labels().end());
            break;
        } else if ((length & 0xC0) == 0x40) {
            // Unrecognized encoding scheme (possibly the one described in
            // now-obsolete RFC 2673 "Binary Labels in the Domain Name System")
            return nullptr;
        } else if ((length & 0xC0) == 0x80) {
            // Unrecognized encoding scheme (possibly the one described in
            // RFC-draft "A New Scheme for the Compression of Domain Names")
            return nullptr;
        } else {
            assert(false);
        }
    } while (length != 0);
    return src;
}
