#include "../include/ReverseProxy.hpp"
#include "../config.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

ReverseProxy::ReverseProxy(const string& internal_server_ip,int internal_server_port): internal_server_ip(internal_server_ip), internal_server_port(internal_server_port){}

int ReverseProxy::create_internal_server_connection(){
    int internal_server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(internal_server_fd < 0){
        cerr<<"Failed to create internal server socket."<<endl;
        return -1;
    }

    struct sockaddr_in internal_server_addr;
    internal_server_addr.sin_family = AF_INET;
    internal_server_addr.sin_port = htons(internal_server_port);

    if(inet_pton(AF_INET,internal_server_ip.c_str(), &internal_server_addr.sin_addr) <= 0){
        cerr<<"Invalid internal server IP address."<<endl;
        close(internal_server_fd);
        return -1;
    }

    if(connect(internal_server_fd,(struct sockaddr*)&internal_server_addr, sizeof(internal_server_addr))<0){
        cerr<<"Connection to internal server failed."<<endl;
        close(internal_server_fd);
        return -1;
    }

    return internal_server_fd;
}

bool ReverseProxy::forward_request(int &internal_server_fd, const char* client_request){
    internal_server_fd = create_internal_server_connection();
    if(internal_server_fd < 0){
        return false;
    }
    
    if(send(internal_server_fd, client_request, strlen(client_request), 0) < 0){
        cerr<<"Failed to send request to backend server."<<endl;
        close(internal_server_fd);
        return false;
    }

    return true;
}

bool ReverseProxy::relay_response(int internal_server_fd, int client_fd){
    if(internal_server_fd < 0){
        return false;
    }

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(internal_server_fd, buffer, sizeof(buffer), 0);
    if(bytes_received < 0){
        cerr<<"Error receiving response from internal server."<<endl;
        close(internal_server_fd);
        return false;
    }

    if(send(client_fd, buffer, bytes_received, 0) < 0){
        cerr<<"Failed to relay response to client."<<endl;
        close(internal_server_fd);
        return false;
    }

    close(internal_server_fd);
    return true;
}


