/*
 * File:   logger.cpp
 * Author: tomas
 *
 * Created on June 30, 2009, 5:18 PM
 */

#include "logger.h"

using namespace dns;

// static
Logger* Logger::_instance = 0;
std::ofstream Logger::_file("dnsserver.log", std::ios::out|std::ios::trunc);

Logger& Logger::instance() throw () {

    if (_instance == 0) {
        _instance == new Logger();
    }

    return *_instance;
}

void Logger::trace(const char* text) throw() {

    _file << " ## " << text << std::endl;
}

void Logger::trace(std::string& text) throw() {

    trace(text.data());
}

void Logger::trace(std::ostringstream& stream) throw() {

    std::string text = stream.str();
    trace(text.data());
}

void Logger::error(const char* text) throw() {

    _file << " !! " << text << std::endl;
}

void Logger::error(std::string& text) throw() {

    trace(text.data());
}
