#include "config.hpp"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        cerr<<"Socket creation failed."<<endl;
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        cerr<<"Bind failed"<<endl;
        return -1;
    }

    if(listen(server_fd, 5) < 0){
        cerr<<"Listen failed"<<endl;
        return -1;
    } 

    cout<<"Internal server is listening to port "<<PORT<<endl;

    while(true){
        int client_fd = accept(server_fd, NULL, NULL);
        if(client_fd < 0){
            cerr<<"Accept failed"<<endl;
            continue;
        }     

        const char* response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 22\n\nHello from internal server!";
        send(client_fd, response, strlen(response),0);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
