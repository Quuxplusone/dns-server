
#include "authoritative-resolver.h"
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
            "Usage: dnsServer <port> <zonefile>\n"
            "Example: dnsServer 9000 zone.txt\n"
        );
    }

    int port = atoi(argv[1]);
    std::string zonefile = argv[2];

    if (port < 1 || port > 65535) {
        exit_with_message("Error: Invalid port number.\n");
    }

    try {
        dns::AuthoritativeResolver resolver(zonefile);
        resolver.print_records();
        dns::Server server(resolver);
        server.bind_to(port);
        std::cout << "Listening on port: " << port << std::endl;
        server.run();
    } catch (const std::exception& e) {
        exit_with_message(e.what());
    }
}
