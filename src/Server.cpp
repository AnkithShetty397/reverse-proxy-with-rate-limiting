#include "../include/Server.hpp"
#include "../config.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(ThreadPool& pool): thread_pool(pool), addrlen(sizeof(address)){
    server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd == 0){
        throw std::runtime_error("Socket creation failed");
    }   

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_fd,(struct sockaddr*)&address,sizeof(address))<0){
        throw std::runtime_error("Bind failed");
    }

    if(listen(server_fd, 3)<0){
        throw std::runtime_error("Listen failed");
    }
    std::cout<<"Server is listening on port"<<PORT<<std::endl;
}

Server::~Server(){
    close(server_fd);
}

void Server::run(){
    while(true){
        int socket_fd = accept(server_fd,(struct sockaddr*)&address,&addrlen);
        if(socket_fd<0){
            std::cerr<<"Accepted failed"<<std::endl;
            continue;
        }
        thread_pool.add_task(socket_fd);
    }
}