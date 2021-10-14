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

using namespace std;

void handleConnection(int socket) {
    cout<<"accepted\n";
    char buffer[100000] = {0};
    int valred = read(socket, buffer, 100000);
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
    string hostname = tokens[1].substr(0, pos);
    int port = stoi(tokens[1].substr(pos+1));
    cout<<hostname<<" "<<port<<"\n";
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