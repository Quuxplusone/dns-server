
#include "bus.h"
#include "message.h"
#include "nonstd.h"
#include "question.h"
#include "rrtype.h"
#include "stub-resolver.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>

void exit_with_message(const char *msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        exit_with_message(
            "Usage: dns-dig <port> <qname> <qtype>\n"
            "Example: dns-dig 9000 google.com. A\n"
        );
    }

    int port = atoi(argv[1]);
    std::string qname_str = argv[2];
    std::string qtype_str = argv[3];

    if (port < 1 || port > 65535) {
        exit_with_message("Error: Invalid port number.\n");
    }

    try {
        bus::Bus bus;
        dns::StubResolver stubresolver(
            bus,
            dns::Upstream("127.0.0.1", port)
        );

        dns::Name qname(qname_str.c_str());
        dns::RRType qtype(qtype_str);
        dns::Question question(qname, qtype, dns::RRClass::IN);
        dns::Message query = dns::Message::beginQuery(std::move(question));

        // Here is where we'd set the query-id or the RD bit, if we wanted to do that.
        query.setRD(true);

        dns::Message response = stubresolver.async_resolve(query, nonstd::seconds(1)).get();

        std::cout << response.repr() << std::endl;
        std::cout << ";; Query time: <FAKE>msec\n" << std::endl;
        std::cout << ";; SERVER: 127.0.0.1#" << port << "(127.0.0.1)" << std::endl;
        std::cout << ";; WHEN: <FAKE>" << std::endl;
        std::cout << ";; MSG SIZE  rcvd: <FAKE>" << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        exit_with_message(e.what());
    }
}
