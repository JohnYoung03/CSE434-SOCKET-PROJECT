#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

int main(int argc, char *argv[])
{
    int portNum = std::stoi(argv[1]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in sockAddr{};

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(portNum);

    bind(sock, (sockaddr *)&sockAddr, sizeof(sockAddr));

    string names[20];
    string ips[20];
    int mPorts[20];
    int pPorts[20];
    string states[20];
    int peerNum = 0;
    
    while(true){
        char mess[1024];

        sockaddr_in peerAddr;
        socklen_t peerAddrSize = sizeof(peerAddr);

        int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&peerAddr, &peerAddrSize);

        mess[messSize] = '\0';

        string command = mess;
        string part[10];
        int partNum = 0;

        for (int i = 0; i < command.length(); i++)
        {
            if (command[i] == ' ')
            {
                partNum++;
            }
            else
            {
                part[partNum] = part[partNum] + command[i];
            }
        }
    }
    return 0;
}
