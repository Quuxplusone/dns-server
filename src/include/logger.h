#pragma once

#include <fstream>

namespace dns {

/**
 *  Logger is a helper class that allows to trace text messages to a log file.
 *  It is a single instance class or also known as Singleton.
 */
class Logger {
public:
    template<class... Args>
    static void trace(Args&&... args) {
        instance().output_with_prefix(" ## ", std::forward<Args>(args)...);
    }

    template<class... Args>
    static void error(Args&&... args) {
        instance().output_with_prefix(" !! ", std::forward<Args>(args)...);
    }

private:
    template<class... Args>
    Logger(Args&&... args) : m_file(std::forward<Args>(args)...) {}

    static Logger& instance() noexcept {
        static Logger instance("dnsserver.log", std::ios::out|std::ios::trunc);
        return instance;
    }

    template<class... Args>
    void output_with_prefix(const char *prefix, Args&&... args) {
        m_file << prefix;
        int a[] = {
            [&]() { m_file << std::forward<Args>(args); return 0; }()...
        };
        (void)a;
        m_file << std::endl;
    }

    std::ofstream m_file;
};

} // namespace dns
