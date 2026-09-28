#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdlib>
#include <ctime>
using namespace std;

int main(int argc, char *argv[])
{
    int portNum = std::stoi(argv[1]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    srand(time(0));

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

    bool dhtExists = false;
    bool waiting = false;
    string leader;
    
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
        if (part[0] == "register")
        {
            string name = part[1];
            string ip = part[2];
            int mPort = stoi(part[3]);
            int pPort = stoi(part[4]);

            bool works = true;

            for (int i = 0; i < peerNum; i++)
            {
                if (names[i] == name)
                {
                    works = false;
                }

                if (mPorts[i] == mPort || mPorts[i] == pPort ||
                    pPorts[i] == mPort || pPorts[i] == pPort)
                {
                    works = false;
                }
            }

            string reply;

            if (works == true)
            {
                names[peerNum] = name;
                ips[peerNum] = ip;
                mPorts[peerNum] = mPort;
                pPorts[peerNum] = pPort;
                states[peerNum] = "Free";

                peerNum = peerNum + 1;

                reply = "SUCCESS";
            }
            else
            {
                reply = "FAILURE";
            }

            sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
        } else if (part[0] == "setup-dht")
        {
            string name = part[1];
            int n = stoi(part[2]);
            int year = stoi(part[3]);

            bool works = true;
            int leaderNum = -1;

            for (int i = 0; i < peerNum; i++)
            {
                if (names[i] == name)
                {
                    leaderNum = i;
                }
            }

            if (leaderNum == -1)
            {
                works = false;
            }

            if (n < 3)
            {
                works = false;
            }

            if (peerNum < n)
            {
                works = false;
            }

            if (dhtExists == true)
            {
                works = false;
            }

            string reply;

            if (works == true)
            {
                states[leaderNum] = "Leader";
                leader = name;
                dhtExists = true;
                waiting = true;

            int picked = 1;

            while (picked < n)
            {
                int randomNum = rand() % peerNum;

                if (randomNum != leaderNum && states[randomNum] == "Free")
                {
                    states[randomNum] = "InDHT";
                    picked = picked + 1;
                }
            }
                reply = "SUCCESS";

                reply = reply + " " + names[leaderNum];
                reply = reply + " " + ips[leaderNum];
                reply = reply + " " + to_string(pPorts[leaderNum]);

                for (int i = 0; i < peerNum; i++)
                {
                    if (states[i] == "InDHT")
                    {
                        reply = reply + " " + names[i];
                        reply = reply + " " + ips[i];
                        reply = reply + " " + to_string(pPorts[i]);
                    }
                }
            }
            else
            {
                reply = "FAILURE";
            }

            sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
        }
    }
    return 0;
}
