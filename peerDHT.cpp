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
    return 0;
}
