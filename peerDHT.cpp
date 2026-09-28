#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

int main(int argc, char *argv[])
{
    string managerIP = argv[1];
    int managerPort = stoi(argv[2]);
    return 0;
}
