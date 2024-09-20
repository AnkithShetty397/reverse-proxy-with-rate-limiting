#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

using namespace std;

int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[1024]={0};
    const char *msg = "Hello from server";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd==0){
        cerr<<"Socket creation failed"<<endl;
    }

    address.sin_family =  AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_fd,(struct sockaddr*)&address, sizeof(address))<0){
        cerr<<"Bind failed"<<endl;
        return -1;
    }
    if(listen(server_fd,3)<0){
        cerr<<"Listen failed"<<endl;
        return -1;
    }
    cout<<"Server is listening to port "<<PORT<<endl;
    new_socket = accept(server_fd, (struct sockaddr*)&address,&addrlen);
    if(new_socket<0){
        cerr<<"Accept failed"<<endl;
        return -1;
    }

    read(new_socket,buffer,1024);
    cout<<"Message received: "<<buffer <<endl;

    send(new_socket, msg, strlen(msg),0);
    cout<<"Message sent to client: "<<msg<<endl;

    close(new_socket);
    close(server_fd);

    return 0;
}