#pragma once

#include <fstream>
#include <sstream>
#include <string>

namespace dns {

/**
 *  Logger is a helper class that allows to trace text messages to a log file.
 *  It is a single instance class or also known as Singleton.
 */
class Logger {
public:
    /**
     *  Instantiates the one and only Logger object.
     *  @return A reference to the one and only Logger object.
     */
    static Logger& instance() noexcept;

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(const char *text) noexcept;

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(const std::string& text) noexcept;

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(const std::ostringstream& text) noexcept;

    /**
     *  Trace the error message to the log file.
     *  @param text Error message to log.
     */
    void error(const char *text) noexcept;

    /**
     *  Trace the error message to the log file.
     *  @param text Error message to log.
     */
    void error(const std::string& text) noexcept;

private:
    template<class... Args>
    Logger(Args&&... args) : m_file(std::forward<Args>(args)...) {}

    ~Logger() = default;

    std::ofstream m_file;
};

} // namespace dns
