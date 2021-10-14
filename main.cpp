#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#define BUFFER_SIZE 500000
using namespace std;
int connectToServer(const char* hostname, const char* port) {
    struct addrinfo hints;
    struct addrinfo *destAddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(hostname, port, &hints, &destAddr);
    int socketfd = socket(destAddr -> ai_family, destAddr -> ai_socktype,
    destAddr -> ai_protocol);
    connect(socketfd, destAddr->ai_addr, destAddr->ai_addrlen);
    cout<<"connected to server!\n";
    freeaddrinfo(destAddr);
    return socketfd;
}
void sendToServer(char* buffer, int socket) {
    
}
void handleConnection(int socket) {
    cout<<"accepted\n";
    char buffer[BUFFER_SIZE] = {0};
    int valred = read(socket, buffer, BUFFER_SIZE);
    cout<<"message:"<<"\n";
    cout<<buffer<<"\n"; 
    stringstream ss;
    ss<<buffer;
    string tok;
    vector<string> tokens;
    while (ss>>tok) {
        tokens.push_back(tok);
    }
    auto pos = tokens[1].find(":");
    const char* hostname = (tokens[1].substr(0, pos)).c_str();
    const char* port = (tokens[1].substr(pos+1)).c_str();
    cout<<hostname<<" "<<port<<"\n";
    int serversocket = connectToServer(hostname, port);

    size_t msglen = recv(serversocket, buffer, BUFFER_SIZE, 0);
    cout<<"replying:"
    cout<<msglen<<":"<<"\n";
    cout<<buffer;
}
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int len = sizeof(address);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( atoi(argv[1]) );
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    while (true) {
        int listenSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&len);
        handleConnection(listenSocket);  
    }
}