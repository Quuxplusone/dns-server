
#include "logger.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace dns;

Logger& Logger::instance() noexcept
{
    static Logger instance("dnsserver.log", std::ios::out|std::ios::trunc);
    return instance;
}

void Logger::trace(const char *text) noexcept
{
    m_file << " ## " << text << std::endl;
}

void Logger::trace(const std::string& text) noexcept
{
    trace(text.data());
}

void Logger::trace(const std::ostringstream& stream) noexcept
{
    std::string text = stream.str();
    trace(text.data());
}

void Logger::error(const char *text) noexcept
{
    m_file << " !! " << text << std::endl;
}

void Logger::error(const std::string& text) noexcept
{
    trace(text.data());
}
