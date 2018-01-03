
#include "resolver.h"
#include "server.h"

#include <iostream>
#include <stdlib.h>
#include <string>

void exit_with_message(const char *msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        exit_with_message(
            "Usage: dnsServer <port> <hostsFile>\n"
            "Example: dnsServer 9000 hosts\n"
        );
    }

    int port = atoi(argv[1]);
    std::string hosts_file = argv[2];

    if (port < 1 || port > 65535) {
        exit_with_message("Error: Invalid port number.\n");
    }

    try {
        dns::Resolver resolver(hosts_file);
        resolver.print_records();
        dns::Server server(resolver);
        server.bind_to(port);
        std::cout << "Listening on port: " << port << std::endl;
        server.run();
    } catch (const std::exception& e) {
        exit_with_message(e.what());
    }
}
