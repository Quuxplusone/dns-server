
#include "exception.h"
#include "upstream.h"

#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

using namespace dns;

Upstream::Upstream(const char *ipv4, int port)
{
    m_sockaddr.sin_family = AF_INET;
    m_sockaddr.sin_addr.s_addr = inet_addr(ipv4);
    m_sockaddr.sin_port = htons(port);
}

int Upstream::bind_udp_socket() const
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throw dns::Exception("Could not open a new socket: ", strerror(errno));
    }
    int rc = bind(sockfd, this->sockaddr(), this->sockaddr_length());
    if (rc != 0) {
        close(sockfd);
        throw dns::Exception("Could not bind: ", strerror(errno));
    }
    return sockfd;
}
