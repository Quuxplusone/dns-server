
#include "application.h"
#include "logger.h"
#include "exception.h"

#include <iostream>
#include <stdlib.h>

using namespace dns;

void Application::parse_arguments(int argc, char **argv)
{
    if (argc != 3) {
        throw dns::Exception(
            "Usage: dnsServer <port> <hostsFile>\n"
            "Example: dnsServer 9000 hosts\n"
        );
    }

    m_port = atoi(argv[1]);
    if (m_port < 1 || m_port > 65535) {
        throw dns::Exception("Error: Invalid port number.\n");
    }

    m_filename = argv[2];
}

void Application::run()
{
    Logger& logger = Logger::instance();
    logger.trace("Application::run()");

    m_resolver.init(m_filename);
    m_server.init(m_port);
    m_server.run();
}
