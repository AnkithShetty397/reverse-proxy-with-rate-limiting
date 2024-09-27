#include "../include/SSLClientHandler.hpp"
#include "../config.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

SSLClientHandler::SSLClientHandler(RateLimiter& limiter, SSLHandler& sslHandler): rate_limiter(limiter), ssl_handler(sslHandler) {}
void SSLClientHandler::handle_client(int socket_fd){
    SSL* ssl = ssl_handler.create_ssl_connection(socket_fd);
    if(!ssl){
        cerr<<"SSL handshake failed."<<endl;
        close(socket_fd);
        return;
    }

    char buffer[BUFFER_SIZE] = {0};
    const char* rate_limit_msg = 
        "HTTP/1.1 429 Too Many Requests\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 20\r\n"
        "\r\n"
        "Rate limit exceeded\n";

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if(getpeername(socket_fd,(struct sockaddr*)&client_addr,&client_addr_len)==0){
        string client_ip = inet_ntoa(client_addr.sin_addr);

        if(rate_limiter.is_rate_limited(client_ip)){
            SSL_write(ssl, rate_limit_msg, strlen(rate_limit_msg));
            cout<<"Blocked: "<<client_ip<<endl;
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(socket_fd);
            return;
        }
    }else{
        cerr<<"Failed to fetch the IP address"<<endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket_fd);
        return;
    }

    cout<<"Accepted SSL connection"<<endl;

    ssize_t bytes_read = SSL_read(ssl, buffer, BUFFER_SIZE);
    if(bytes_read < 0){
        cerr<<"Error reading from SSL socket"<<endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket_fd);
        return;
    }

    int internal_server_fd;
    ReverseProxy proxy(INTERNAL_SERVER_IP, INTERNAL_SERVER_PORT);
    if(!proxy.forward_request(internal_server_fd, buffer)){
        cerr<<"Failed to forward request to the internal server."<<endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket_fd);
        return;
    }

    char response_buffer[BUFFER_SIZE] = {0};
    ssize_t response_bytes = recv(internal_server_fd, response_buffer, BUFFER_SIZE, 0);
    if(response_bytes < 0){
        cerr<<"Failed to receive response from internal server."<<endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket_fd);
        close(internal_server_fd);
        return;
    }

    if(SSL_write(ssl, response_buffer, response_bytes) < 0){
        cerr<<"Failed to relay response to client."<<endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(socket_fd);
        close(internal_server_fd);
        return;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(internal_server_fd);
    close(socket_fd);
}