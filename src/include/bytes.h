#pragma once

#include <inttypes.h>
#include <string>

namespace dns {

const char *get8bits(const char *src, const char *end, uint8_t& out) noexcept;
const char *get16bits(const char *src, const char *end, uint16_t& out) noexcept;
const char *get32bits(const char *src, const char *end, uint32_t& out) noexcept;
const char *get_uint8_sized_string(const char *src, const char *end, std::string& out);
const char *get_uint16_sized_string(const char *src, const char *end, std::string& out);

char *put8bits(char *dst, const char *end, uint8_t value) noexcept;
char *put16bits(char *dst, const char *end, uint16_t value) noexcept;
char *put32bits(char *dst, const char *end, uint32_t value) noexcept;
char *put_uint8_sized_string(char *dst, const char *end, const std::string& s) noexcept;
char *put_uint16_sized_string(char *dst, const char *end, const std::string& s) noexcept;

} // namespace dns
