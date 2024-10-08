#ifndef SSLCLIENTHANDLER_HPP
#define SSLCLIENTHANDLER_HPP

#include "RateLimiter.hpp"
#include "SSLHandler.hpp"
#include "ReverseProxy.hpp"

class SSLClientHandler{
private:
    RateLimiter& rate_limiter;
    SSLHandler& ssl_handler;

public:
    SSLClientHandler(RateLimiter& limiter, SSLHandler& sshandler);
    void handle_client(int socket_fd);
};

#endif