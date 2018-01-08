
#include "bytes.h"

#include <assert.h>
#include <inttypes.h>
#include <string>
#include <string.h>

namespace dns {

const char *get8bits(const char *src, const char *end, uint8_t& out) noexcept
{
    if (src == nullptr || src == end) return nullptr;
    out = static_cast<uint8_t>(*src++);
    return src;
}

const char *get16bits(const char *src, const char *end, uint16_t& out) noexcept
{
    uint16_t value = 0;
    if (src == nullptr || (end - src) < 2) return nullptr;
    value = (value << 8) | static_cast<uint8_t>(*src++);
    value = (value << 8) | static_cast<uint8_t>(*src++);
    out = value;
    return src;
}

const char *get32bits(const char *src, const char *end, uint32_t& out) noexcept
{
    uint32_t value = 0;
    if (src == nullptr || (end - src) < 4) return nullptr;
    value = (value << 8) | static_cast<uint8_t>(*src++);
    value = (value << 8) | static_cast<uint8_t>(*src++);
    value = (value << 8) | static_cast<uint8_t>(*src++);
    value = (value << 8) | static_cast<uint8_t>(*src++);
    out = value;
    return src;
}

const char *get_uint8_sized_string(const char *src, const char *end, std::string& out)
{
    uint8_t length;
    src = get8bits(src, end, length);
    if (src == nullptr || (end - src) < length) return nullptr;
    out.resize(length);
    memcpy(&out[0], src, length);
    src += length;
    return src;
}

const char *get_uint16_sized_string(const char *src, const char *end, std::string& out)
{
    uint16_t length;
    src = get16bits(src, end, length);
    if (src == nullptr || (end - src) < length) return nullptr;
    out.resize(length);
    memcpy(&out[0], src, length);
    src += length;
    return src;
}

char *put8bits(char *dst, const char *end, uint8_t value) noexcept
{
    if (dst == nullptr || dst == end) return nullptr;
    *dst++ = static_cast<uint8_t>(value);
    return dst;
}

char *put16bits(char *dst, const char *end, uint16_t value) noexcept
{
    if (dst == nullptr || (end - dst) < 2) return nullptr;
    *dst++ = static_cast<uint8_t>(value >> 8);
    *dst++ = static_cast<uint8_t>(value >> 0);
    return dst;
}

char *put32bits(char *dst, const char *end, uint32_t value) noexcept
{
    if (dst == nullptr || (end - dst) < 4) return nullptr;
    *dst++ = static_cast<uint8_t>(value >> 24);
    *dst++ = static_cast<uint8_t>(value >> 16);
    *dst++ = static_cast<uint8_t>(value >> 8);
    *dst++ = static_cast<uint8_t>(value >> 0);
    return dst;
}

static char *putstring(char *dst, const char *end, const std::string& value) noexcept
{
    if (dst == nullptr || (end - dst) < value.size()) return nullptr;
    memcpy(dst, value.data(), value.size());
    dst += value.size();
    return dst;
}

char *put_uint8_sized_string(char *dst, const char *end, const std::string& value) noexcept
{
    assert(value.size() <= 255);
    if (dst == nullptr || (end - dst) < value.size() + 1) return nullptr;
    dst = put8bits(dst, end, value.size());
    dst = putstring(dst, end, value);
    return dst;
}

char *put_uint16_sized_string(char *dst, const char *end, const std::string& value) noexcept
{
    assert(value.size() <= 65535);
    if (dst == nullptr || (end - dst) < value.size() + 2) return nullptr;
    dst = put16bits(dst, end, value.size());
    dst = putstring(dst, end, value);
    return dst;
}

} // namespace dns
