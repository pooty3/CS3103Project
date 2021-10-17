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

// constant definitions
#define BUFFER_SIZE 500000


using namespace std;
void print(char* p, int len) {
    for (int i =0; i < len; i++) {
        printf("%c", p[i]);
    }
}
int connectToServer(const char* hostname, const char* port) {
    printf("Host name is %s, port is %s\n", hostname, port);
    struct addrinfo hints;
    struct addrinfo *destAddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, port, &hints, &destAddr) != 0) {
        printf("Error in server address format!\n"); 
        exit(1);
    };
    int socketfd = socket(destAddr -> ai_family, destAddr -> ai_socktype,
    destAddr -> ai_protocol);
    if (connect(socketfd, destAddr->ai_addr, destAddr->ai_addrlen) < 0) {
        printf("Failed to connect to %s:%s", hostname,port);
        exit(1);
    } 
    cout<<"connected to server!\n";
    freeaddrinfo(destAddr);
    return socketfd;
}

void sendToServer(char* sendBuffer, int socket, int length) {
    //printf("payload: \n%s", sendBuffer);
    if (send(socket, (void *) sendBuffer, length, 0) < 0) {
        printf("Error in sending!\n");
        exit(1);
    };
    cout<<"sending\n";
}

void handleClientRequest(int socket) {
    cout<<"accepted\n";
    char buffer[BUFFER_SIZE];
    int valred = read(socket, buffer, BUFFER_SIZE);
    cout<<buffer<<"\n"; 
    stringstream ss;
    ss<<buffer;
    string tok;
    vector<string> tokens;

    while (ss>>tok) {
        tokens.push_back(tok);
    }
    auto pos = tokens[1].find(":");
    printf("pos is %d\n", pos);
    string s1 = tokens[1].substr(0, pos);
    const char* hostname = s1.c_str();
    const char* port = (tokens[1].substr(pos+1)).c_str();
    //string s1 = tokens[1].substr(7, 15);
    //const char* hostname = s1.c_str();
   // string pp = "80";
    //const char* port = pp.c_str();
    printf("hostname: %s and port is %s\n", hostname, port);
    int serversocket = connectToServer(hostname, port);
    
    sendToServer(buffer, serversocket, valred);
    cout<<"sendingover!\n";
    
    char returnBuffer[BUFFER_SIZE] = {0};
    int iRec;
    while ((iRec = recv(serversocket, returnBuffer, BUFFER_SIZE, 0)) > 0) {
        print(returnBuffer, iRec);
        printf("\nreceive: %d of:\n%s", iRec, returnBuffer);
        if (send(socket, (void *)returnBuffer, iRec, 0) <0) {
            printf("Error sending back\n");
            exit(1);
        };
        printf("sent!\n");
        memset(returnBuffer, 0, sizeof(returnBuffer));
    }
    printf("done receiving!\n");
    if (iRec < 0) {
        printf("Error!\n"); 
        exit(1);
    }
    //size_t msglen = recv(serversocket, returnBuffer, BUFFER_SIZE, 0);
    //cout<<"returned\n";
    //cout<<returnBuffer<<"\n";
    close(socket);
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
        handleClientRequest(listenSocket);  
    }
}