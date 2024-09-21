#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include "RateLimiter.hpp"

class ClientHandler{
private:
    RateLimiter& rate_limiter;

public:
    ClientHandler(RateLimiter& limiter);
    void handle_client(int socket_fd);
};

#endif 