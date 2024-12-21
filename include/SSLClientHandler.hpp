#ifndef SSLCLIENTHANDLER_HPP
#define SSLCLIENTHANDLER_HPP

#include "RateLimiter.hpp"
#include "SSLHandler.hpp"
#include "ConnectionPool.hpp"

class SSLClientHandler{
private:
    RateLimiter& rate_limiter;
    SSLHandler& ssl_handler;

public:
    SSLClientHandler(RateLimiter& limiter, SSLHandler& sshandler);
    void handle_client(int socket_fd);
    string extract_url(const char* buffer);
};

#endif