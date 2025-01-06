#include "./RedisConnection.cpp"
#include "../include/ConnectionPool.hpp"
#include "../include/SSLClientHandler.hpp"
#include "../config.hpp"

#include <hiredis/hiredis.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

string SSLClientHandler::extract_url(const char* buffer){
    const char* method = "GET";
    const char* http_version = " HTTP/1.1";

    if(strncmp(buffer, method, strlen(method)) == 0){
        const char* start = buffer + strlen(method);
        const char* end = strstr(start, http_version);

        if(end != NULL){
            return string(start, end-start);
        }
    }

    return "";
}

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
    
    int internal_server_fd;

    while(true){
        ssize_t bytes_read = SSL_read(ssl,buffer,BUFFER_SIZE);
        if(bytes_read <= 0){
            if(bytes_read == 0 || SSL_get_error(ssl,bytes_read) == SSL_ERROR_ZERO_RETURN){
                cout<<"Client closed connection."<<endl;
            }else{
                cerr<<"Error reading from the SSL socket."<<endl;
            }
            break;
        }
        
        buffer[bytes_read]='\0';

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        if(getpeername(socket_fd,(struct sockaddr*)&client_addr,&client_addr_len)==0){
            string client_ip = inet_ntoa(client_addr.sin_addr);
            int client_port = ntohs(client_addr.sin_port);

            if(rate_limiter.is_rate_limited(client_ip)){
                SSL_write(ssl, rate_limit_msg, strlen(rate_limit_msg));
                cout<<"Blocked: "<<client_ip<<":"<<client_port<<endl;
                continue;
            }
        }else{
            cerr<<"Failed to fetch the IP address"<<endl;
            break;
        }

        RedisConnection& redis_conn = RedisConnection::getInstance();
        string key = extract_url(buffer);
        string cached_response = redis_conn.getCache(key);

        if(!cached_response.empty()){
            SSL_write(ssl, cached_response.c_str(), cached_response.size());
        }else{
            ConnectionPool& pool = ConnectionPool::getInstance();
            int internal_server_fd = pool.getConnection();

            if(internal_server_fd < 0){
                cerr<<"Failed to acquire a connection fron the pool."<<endl;
                break;
            }

            if(send(internal_server_fd, buffer, strlen(buffer), 0) <= 0){
                cerr<<"Failed to send request to the internal server."<<endl;
                pool.releaseConnection(internal_server_fd);
                break;
            }

            char response_buffer[BUFFER_SIZE] = {0};
            ssize_t response_bytes = recv(internal_server_fd, response_buffer, BUFFER_SIZE, 0);
            if(response_bytes <= 0){
                if (response_bytes == 0) {
                    cerr << "Internal server closed the connection." << endl;
                } else {
                    cerr << "Failed to receive response from internal server. Error: " << strerror(errno) << endl;
                }
                pool.releaseConnection(internal_server_fd);
                break;
            }

            if (response_bytes > 0) {
                if (SSL_write(ssl, response_buffer, response_bytes) < 0) {
                    cerr << "Failed to relay response to client." << endl;
                    break;
                }
            }

            if(key != ""){
                redis_conn.setCache(key, string(response_buffer, response_bytes));
                cout<<"Cached '"<<key<<"'"<<endl;
            }

            pool.releaseConnection(internal_server_fd);
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socket_fd);
}