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
    string waitingFor = "";
    string leader;
    string leavingPeer = "";
    string joiningPeer = "";
    
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

        if (waiting == true && part[0] != waitingFor)
        {
            string reply = "FAILURE";

            sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
        }
        else if (part[0] == "register")
        {
            string name = part[1];
            string ip = part[2];
            int mPort = stoi(part[3]);
            int pPort = stoi(part[4]);

            bool works = true;
            if (mPort == pPort)
            {
                works = false;
            }

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
                waitingFor = "dht-complete";

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
        else if (part[0] == "dht-complete")
        {
            string name = part[1];
            string reply;

            if (name == leader)
            {
                waiting = false;
                waitingFor = "";
                reply = "SUCCESS";
            }
            else
            {
                reply = "FAILURE";
            }

            sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
        }
        else if(part[0] == "query-dht"){
            string name = part[1];
            bool isGood = false;
            for (int i = 0; i < peerNum; i++){
                if (names[i] == name && states[i] == "Free"){
                    isGood = true;
                }
            }
            string reply;
                if (dhtExists == false)
                {
                    isGood = false;
                }

                if (isGood == true)
                {
                    int randomNum = rand() % peerNum;

                    while (states[randomNum] != "Leader" && states[randomNum] != "InDHT")
                    {
                        randomNum = rand() % peerNum;
                    }

                    reply = "SUCCESS";
                    reply = reply + " " + names[randomNum];
                    reply = reply + " " + ips[randomNum];
                    reply = reply + " " + to_string(pPorts[randomNum]);
                 }
                else
                {
                    reply = "FAILURE";
                }

                sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
            }
            else if (part[0] == "deregister"){
            string name = part[1];
            int peerIndex = -1;

            for(int i = 0; i < peerNum; i++){
                if(names[i] == name){
                    peerIndex = i;
                 }
            }

            string reply;

            if(peerIndex == -1){
                reply = "FAILURE";
            }
            else if(states[peerIndex] != "Free"){
                reply = "FAILURE";
            }
            else{
                for(int i = peerIndex; i < peerNum - 1; i++){
                    names[i] = names[i + 1];
                    ips[i] = ips[i + 1];
                    mPorts[i] = mPorts[i + 1];
                    pPorts[i] = pPorts[i + 1];
                    states[i] = states[i + 1];
                }

                peerNum = peerNum - 1;
                reply = "SUCCESS";
            }

            sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
        }
        else if(part[0] == "teardown-dht"){
        string name = part[1];
        string reply;

        if(name != leader || dhtExists == false){
            reply = "FAILURE";
        }
        else{
            reply = "SUCCESS";
            waiting = true;
            waitingFor = "teardown-complete";
        }    

        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
    }
    else if(part[0] == "teardown-complete"){
        string name = part[1];
        string reply;

        if(name != leader){
            reply = "FAILURE";
        }
        else{
            for(int i = 0; i < peerNum; i++){
                if(states[i] == "Leader" || states[i] == "InDHT"){
                    states[i] = "Free";
                }
            }

            dhtExists = false;
            waiting = false;
            waitingFor = "";
            leader = "";
            reply = "SUCCESS";
        }

        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
    }
    else if(part[0] == "leave-dht"){
        string name = part[1];
        bool isGood = false;

        for(int i = 0; i < peerNum; i++){
            if(names[i] == name && (states[i] == "Leader" || states[i] == "InDHT")){
                isGood = true;
            }
        }

        string reply;

        if(dhtExists == false){
            isGood = false;
        }

        if(isGood == true){
            reply = "SUCCESS";
            waiting = true;
            waitingFor = "dht-rebuilt";
            leavingPeer = name;
        }
        else{
            reply = "FAILURE";
        }

        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
    }
    else if(part[0] == "join-dht"){
        string name = part[1];
        int joinNum = -1;
        int leaderNum = -1;

        for(int i = 0; i < peerNum; i++){
            if(names[i] == name){
                joinNum = i;
            }

            if(names[i] == leader){
                leaderNum = i;
            }
        }

        string reply;

        if(joinNum == -1 || states[joinNum] != "Free" || dhtExists == false || leaderNum == -1){
            reply = "FAILURE";
        }    
        else{
            reply = "SUCCESS";
            reply = reply + " " + names[leaderNum];
            reply = reply + " " + ips[leaderNum];
            reply = reply + " " + to_string(pPorts[leaderNum]);

            waiting = true;
            waitingFor = "dht-rebuilt";
            joiningPeer = name;
        }

        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
    }
    else if(part[0] == "dht-rebuilt"){
        string name = part[1];
        string newLeader = part[2];
        int newLeaderNum = -1;

        for(int i = 0; i < peerNum; i++){
            if(names[i] == newLeader){
                newLeaderNum = i;
            }
        }

        string reply;

        if(newLeaderNum == -1){
        reply = "FAILURE";
        }
        else if(leavingPeer != "" && name == leavingPeer){
            for(int i = 0; i < peerNum; i++){
                if(names[i] == leavingPeer){
                    states[i] = "Free";
                }
                else if(names[i] == newLeader){
                    states[i] = "Leader";
                }
                else if(states[i] == "Leader" || states[i] == "InDHT"){
                    states[i] = "InDHT";
                }
            }

            leader = newLeader;
            waiting = false;
            waitingFor = "";
            leavingPeer = "";
            reply = "SUCCESS";
        }
        else if(joiningPeer != "" && name == joiningPeer){
            for(int i = 0; i < peerNum; i++){
                if(names[i] == joiningPeer){
                    states[i] = "InDHT";
                }
                else if(names[i] == newLeader){
                    states[i] = "Leader";
                }
                else if(states[i] == "Leader" || states[i] == "InDHT"){
                    states[i] = "InDHT";
                }
            }

            leader = newLeader;
            waiting = false;
            waitingFor = "";
            joiningPeer = "";
            reply = "SUCCESS";
        }
        else{
            reply = "FAILURE";
        }

        sendto(sock, reply.c_str(), reply.size(), 0, (sockaddr *)&peerAddr, peerAddrSize);
    }  
            
        }
    return 0;
}
