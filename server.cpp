#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <arpa/inet.h>

#define PORT 8080

using namespace std;

void handle_client(int socket_fd){
    char buffer[1024] = {0};
    const char *msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-length: 19\n\nHello from server";
    
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if(getpeername(socket_fd,(struct sockaddr*)&client_addr, &client_addr_len)==0){
        char *client_ip = inet_ntoa(client_addr.sin_addr);
        cout<<"Client ip address: "<<client_ip<<endl;
    }else{
        cerr<<"Failed to fetch the IP address"<<endl;
    }

    read(socket_fd,buffer,1024);
    cout<<"Message received: "<<buffer<<endl;

    send(socket_fd,msg,strlen(msg),0);
    cout<<"HTTP message sent to client: "<<msg<<endl;

    
    close(socket_fd);
}

int main(){
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        cerr<<"Socket creation failed"<<endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_fd,(struct sockaddr*)&address, sizeof(address))<0){
        cerr<<"Build failed"<<endl;
        return -1;
    }

    if(listen(server_fd,3)<0){
        cerr<<"Listen failed"<<endl;
        return -1;
    }
    cout<<"Server is listening to port "<<PORT<<endl;

    while(true){
        int socket_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if(socket_fd < 0){
            cerr<<"Accept failed"<<endl;
            continue;
        } 
        thread client_thread(handle_client, socket_fd);
        client_thread.detach();
    }

    close(server_fd);

    return 0;
}