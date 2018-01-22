#pragma once

#include "authoritative-resolver.h"

namespace dns {

/**
 *  Server class is a socket server that receives queries and responds to
 *  those queries.
 */
class Server {
public:
    /**
     *  Constructor.
     *  Creates a socket Server.
     *  @param resolver The object @ref Resolver from the application.
     */
    explicit Server(AuthoritativeResolver& resolver) : m_resolver(resolver) {}

    /**
     *  Initializes the server creating a UDP datagram socket and binding it to
     *  the INADDR_ANY address and the port passed.
     *  @param port Port number where the socket is to be bound.
     */
    void bind_to(int port);

    /**
     *  The socket server runs in an infinite loop, waiting for queries and
     *  handling them through the @ref Resolver and sending back the responses.
     */
    void run() noexcept;

private:
    int m_sockfd;
    AuthoritativeResolver& m_resolver;
};

} // namespace dns
