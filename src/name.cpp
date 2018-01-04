
#include "exception.h"
#include "name.h"

#include <ctype.h>
#include <string>
#include <string.h>

using namespace dns;

static bool can_appear_unquoted_in_label(char ch)
{
    return isalnum(ch) || (ch == '-') || (ch == '_') || (ch == '.') || (ch == '\\');
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
    std::string current_label;
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
            throw dns::Exception("unusual name " + std::string(repr, p) + " requires double quotes");
        } else {
            current_label += (*p);
            if (current_label.size() > 63) {
                throw dns::Exception("name contains a label longer than 63 characters");
            }
        }
        ++p;
    }
    if (current_label != "") {
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

static std::string repr_of_label(const std::string& label)
{
    std::string result;
    for (char ch : label) {
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
        result += repr_of_label(label);
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

const char *Name::decode(const char *src, const char *end)
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
            throw dns::UnsupportedException("name field uses POINTER encoding scheme");
        } else {
            throw dns::Exception("name field uses an unsupported encoding scheme");
        }
    } while (length != 0);
    return src;
}
