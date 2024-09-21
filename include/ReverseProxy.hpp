#ifndef REVERSEPROXY_HPP
#define REVERSEPROXY_HPP

#include <string>

class ReverseProxy{
public:
    ReverseProxy(const std::string& internal_server_ip, int internal_server_port);
    bool forward_request(int& internal_server_fd, const char* client_request);
    bool relay_response(int internal_server_fd,int client_socket_fd);

private:
    std::string internal_server_ip;
    int internal_server_port;
    int create_internal_server_connection();

};

#endif