#include "../include/ClientHandler.hpp"
#include "../config.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

ClientHandler::ClientHandler(RateLimiter& limiter): rate_limiter(limiter){}

void ClientHandler::handle_client(int socket_fd){
    char buffer[BUFFER_SIZE]={0};
    const char* msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-length: 19\n\nHello from server";
    const char* rate_limit_msg = "HTTP/1.1 429 Too Many Requests\nContent-Type: text/plain\nContent-length: 30\n\nRate limit exceeded";

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if(getpeername(socket_fd,(struct sockaddr*)&client_addr, &client_addr_len)==0){
        string client_ip = inet_ntoa(client_addr.sin_addr);

        if(rate_limiter.is_rate_limited(client_ip)){
            send(socket_fd, rate_limit_msg,strlen(rate_limit_msg),0);
            cout<<"Blocked: "<<client_ip<<endl;
            close(socket_fd);
            return;
        }
    }else{
        cerr<<"Failed to fetch the IP address"<<endl;
        close(socket_fd);
        return;
    }

    ssize_t bytes_read = read(socket_fd,buffer,BUFFER_SIZE);
    if(bytes_read<0){
        cerr<<"Error reading from socket"<<endl;
        close(socket_fd);
        return;
    }
    cout<<"Message received: "<<buffer<<endl;

    ssize_t bytes_sent = send(socket_fd,msg,strlen(msg),0);
    if(bytes_sent<0){
        cerr<<"Error sending response to client"<<endl;
    }else{
        cout<<"Response sent to the client"<<endl;
    }

    close(socket_fd);
}