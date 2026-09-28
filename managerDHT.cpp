#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

int main(int argc, char *argv[])
{
    string names[20];
    string ips[20];
    int mPorts[20];
    int pPorts[20];
    string states[20];
    int peerNum = 0;
    
    while(true){
        
    }
    int portNum = std::stoi(argv[1]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in sockAddr{};

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(portNum);

    bind(sock, (sockaddr *)&sockAddr, sizeof(sockAddr));
    return 0;
}
