
#include "digger.h"
#include "message.h"
#include "question.h"
#include "rrtype.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>

struct AsyncScheduler {
    template<class Task>
    auto operator()(Task&& task) -> decltype(std::async(std::forward<Task>(task))) {
        return std::async(std::forward<Task>(task));
    }
};

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
        AsyncScheduler scheduler;
        dns::Digger digger(scheduler);

        dns::Name qname(qname_str.c_str());
        dns::RRType qtype(qtype_str);
        dns::Question q(qname, qtype, dns::RRClass::IN);

        dns::Message response = digger.dig(q).get();

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
