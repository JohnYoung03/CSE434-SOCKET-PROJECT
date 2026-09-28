#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

int main(int argc, char *argv[])
{
    string managerIP = argv[1];
    int managerPort = stoi(argv[2]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in manAdd{};
    manAdd.sin_family = AF_INET;
    manAdd.sin_port = htons(manPort);
    net_pton(AF_INET, manIP.c_str(), &manAdd.sin_addr);
    return 0;
}
