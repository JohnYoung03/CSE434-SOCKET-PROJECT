
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
using namespace std;
void peerMessage(int pSock, string message, string ip, int port)
{
    sockaddr_in toAdd{};
    toAdd.sin_family = AF_INET;
    toAdd.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &toAdd.sin_addr);

    sendto(pSock, message.c_str(), message.size(), 0, (sockaddr *)&toAdd, sizeof(toAdd));
}

int main(int argc, char *argv[])
{
    string manIP = argv[1];
    int manPort = stoi(argv[2]);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int pSock = socket(AF_INET, SOCK_DGRAM, 0);

    int f = fcntl(pSock, F_GETFL, 0);
    fcntl(pSock, F_SETFL, f | O_NONBLOCK);

    int inFlag = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, inFlag | O_NONBLOCK);
    
    sockaddr_in manAdd{};
    manAdd.sin_family = AF_INET;
    manAdd.sin_port = htons(manPort);
    inet_pton(AF_INET, manIP.c_str(), &manAdd.sin_addr);
    
    sockaddr_in peerAdd{};
    peerAdd.sin_family = AF_INET;
    peerAdd.sin_addr.s_addr = INADDR_ANY;

    string names[20];
    string ips[20];
    int pPorts[20];
    int id = -1;
    int ringSize = 0;
    string rightNebIP;
    int rightNebPort = 0;
    int s = 0;
	int dhtYear = 0;
    string hashTable[50000];
    int totalRecords[20] = {};
    string myName;
    string myIP;
    int myPPort = 0;
    
    while(true)
    {
        char input[1024];
        int inputSize = read(0, input, 1023);

        if (inputSize > 0)
        {
            input[inputSize] = '\0';

            string command = input;
            string part[10];
            int partNum = 0;

            for (int i = 0; i < command.length(); i++)
            {
                if (command[i] == ' ')
                {
                    partNum++;
                }
                else if (command[i] != '\n')
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
                myName = name;
                myIP = ip;
                myPPort = pPort;
                
                peerAdd.sin_port = htons(mPort);
                bind(sock, (sockaddr *)&peerAdd, sizeof(peerAdd));

                sockaddr_in pAdd{};
                pAdd.sin_family = AF_INET;
                pAdd.sin_addr.s_addr = INADDR_ANY;
                pAdd.sin_port = htons(pPort);
                bind(pSock, (sockaddr *)&pAdd, sizeof(pAdd));
                
                sendto(sock, command.c_str(), command.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));
                char mess[1024];

                sockaddr_in replyAdd;
                socklen_t replyAddSize = sizeof(replyAdd);

                 int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);

                 mess[messSize] = '\0';

                 cout << mess << endl;
            }
            else if (part[0] == "setup-dht")
            {
                string name = part[1];
                int n = stoi(part[2]);
                int year = stoi(part[3]);
				
                sendto(sock, command.c_str(), command.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));

                char mess[1024];

                sockaddr_in replyAdd;
                socklen_t replyAddSize = sizeof(replyAdd);

                int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);

                mess[messSize] = '\0';

                cout << mess << endl;

                string reply = mess;
                string replyPart[100];
                int replyNum = 0;

                for (int i = 0; i < reply.length(); i++)
                {
                    if (reply[i] == ' ')
                    {
                        replyNum++;
                    }
                    else
                    {
                        replyPart[replyNum] = replyPart[replyNum] + reply[i];
                    }
                }

                if (replyPart[0] == "SUCCESS")
                {
					dhtYear = year;
                    id = 0;
                    ringSize = n;

                    for (int i = 0; i < n; i++)
                    {
                        names[i] = replyPart[1 + i * 3];
                        ips[i] = replyPart[2 + i * 3];
                        pPorts[i] = stoi(replyPart[3 + i * 3]);
                    }
                    rightNebIP = ips[(id + 1) % ringSize];
                    rightNebPort = pPorts[(id + 1) % ringSize];
                    string fileName = "details-" + to_string(year) + ".csv";
                    ifstream file(fileName);
                    string line;
                    int count = 0;
                    while(getline(file, line)){
                        if(line.find("event_id") == string::npos){
                            count = count +1;
                        }
                    }
                    bool isPrime = false;
                    int currN = 2 * count+1;
                    while(isPrime != true){
                        isPrime = true;
                        for(int i=2; i < currN; i++){
                            if(currN % i ==0){
                                isPrime=false;
                            }
                        }
                        if(isPrime ==true){
                            s = currN;
                        }
                        else{
                            currN = currN+1;
                        }
                    }
                    
                    
                    for (int i = 1; i < n; i++)
                    {
                        string setMess = "set-id " + to_string(i) + " " + to_string(n);

                        for (int j = 0; j < n; j++)
                        {
                            setMess = setMess + " " + names[j];
                            setMess = setMess + " " + ips[j];
                            setMess = setMess + " " + to_string(pPorts[j]);
                        }
						
						setMess = setMess + " " + to_string(dhtYear);
                        sockaddr_in toAdd{};
                        toAdd.sin_family = AF_INET;
                        toAdd.sin_port = htons(pPorts[i]);
                        inet_pton(AF_INET, ips[i].c_str(), &toAdd.sin_addr);

                        sendto(pSock, setMess.c_str(), setMess.size(), 0, (sockaddr *)&toAdd, sizeof(toAdd));
                    }
                    cout << "Waiting 2 seconds" << endl;
                    sleep(2);
                    string sizeMess = "set-size " + to_string(s);
                    peerMessage(pSock, sizeMess, rightNebIP, rightNebPort);
                    file.close();
                    file.open(fileName);

                    while(getline(file, line)){
                        if(line.find("event_id") == string::npos){
                            int comma = line.find(',');
                            string event = line.substr(0, comma);
                            int eventID = stoi(event);
                            int pos = eventID % s;
                            int peerID = pos % ringSize;
                            totalRecords[peerID] = totalRecords[peerID] + 1;
                            if (peerID == id){
                                hashTable[pos] = line;
                            }
                            else{
                                string storeMess = "store " + to_string(peerID) + " " + to_string(pos) + " " + line;
                                peerMessage(pSock, storeMess, rightNebIP, rightNebPort);
                            }
                        }
                    }  
                    cout << "Waiting 2 seconds" << endl;
                    sleep(2);
                    for(int i = 0; i < ringSize; i++){
                        cout << "Peer " << i << ": " << totalRecords[i] << " records" << endl;
                    }
                    string completeMess = "dht-complete " + name;
                    sendto(sock, completeMess.c_str(), completeMess.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));

                    char completeReply[1024];

                    sockaddr_in replyAdd;
                    socklen_t replyAddSize = sizeof(replyAdd);

                    int completeSize = recvfrom(sock, completeReply, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);

                    completeReply[completeSize] = '\0';

                    cout << completeReply << endl;
                }
            }
            else if(part[0] =="query-dht"){
                string name = part[1];
                int eventID = stoi(part[2]);
                string queryMess = "query-dht " + name;
                sendto(sock, queryMess.c_str(), queryMess.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));
                char mess[1024];
                sockaddr_in replyAdd;
                socklen_t replyAddSize = sizeof(replyAdd);
                int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);
                mess[messSize] = '\0';
                cout << mess << endl;
                string reply = mess;
                string replyPart[10];
                int replyNum = 0;
                for (int i = 0; i < reply.length(); i++)
                {
                    if (reply[i] == ' ')
                    {
                        replyNum++;
                    }
                    else{
                        replyPart[replyNum] = replyPart[replyNum] + reply[i];
                    }
                 }

                if (replyPart[0] == "SUCCESS"){
                    string dhtName = replyPart[1];
                    string dhtIP = replyPart[2];
                    int dhtPort = stoi(replyPart[3]);

                    string findMess = "find-event " + to_string(eventID) + " " + myName + " " + myIP+ " " + to_string(myPPort) + " 0";
                    peerMessage(pSock, findMess, dhtIP, dhtPort);
                }
                
            }
			else if(part[0] == "deregister"){
    		string name = part[1];
    		sendto(sock, command.c_str(), command.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));

    		char mess[1024];
    		sockaddr_in replyAdd;
    		socklen_t replyAddSize = sizeof(replyAdd);
    		int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);
    		mess[messSize] = '\0';

    		cout << mess << endl;

    		string reply = mess;

    		if(reply == "SUCCESS"){
        		close(sock);
        		close(pSock);
        		return 0;
    		}
		}
		else if(part[0] == "teardown-dht"){
    	sendto(sock, command.c_str(), command.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));

    	char mess[1024];
   	 	sockaddr_in replyAdd;
    	socklen_t replyAddSize = sizeof(replyAdd);
    	int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);
    	mess[messSize] = '\0';

    	cout << mess << endl;

    	string reply = mess;

    	if(reply == "SUCCESS"){
     	   string teardownMess = "teardown";
     	   peerMessage(pSock, teardownMess, rightNebIP, rightNebPort);
    	}
	 }
    }

        char peerMess[1024];

        sockaddr_in fromAdd;
        socklen_t fromAddSize = sizeof(fromAdd);

        int peerMessSize = recvfrom(pSock, peerMess, 1023, 0, (sockaddr *)&fromAdd, &fromAddSize);

        if (peerMessSize > 0)
        {
            peerMess[peerMessSize] = '\0';

            string peerCommand = peerMess;
            string peerPart[100];
            int peerPartNum = 0;

            for (int i = 0; i < peerCommand.length(); i++)
            {
                if (peerCommand[i] == ' ')
                {
                    peerPartNum++;
                }
                else
                {
                    peerPart[peerPartNum] = peerPart[peerPartNum] + peerCommand[i];
                }
            }

            if (peerPart[0] == "set-id")
            {
                id = stoi(peerPart[1]);
                ringSize = stoi(peerPart[2]);
                for (int i = 0; i < ringSize; i++)
                {
                    names[i] = peerPart[3 + i * 3];
                    ips[i] = peerPart[4 + i * 3];
                    pPorts[i] = stoi(peerPart[5 + i * 3]);
                }
				dhtYear = stoi(peerPart[3 + ringSize * 3]);
                rightNebIP = ips[(id + 1) % ringSize];
                rightNebPort = pPorts[(id + 1) % ringSize];

            }
            else if (peerPart[0] == "set-size")
            {
                s = stoi(peerPart[1]);

                if (id != 0)
                {
                    peerMessage(pSock, peerCommand, rightNebIP, rightNebPort);
                }
            } else if (peerPart[0] == "store")
                {
                    int storeID = stoi(peerPart[1]);
                    int pos = stoi(peerPart[2]);
                    int firstPart = peerCommand.find(' ');
                    int secondPart = peerCommand.find(' ', firstPart + 1);
                    int thirdPart = peerCommand.find(' ', secondPart + 1);
                    string record = peerCommand.substr(thirdPart + 1);
                    if(storeID == id){
                        hashTable[pos] = record;
                    }
                    else{
                        peerMessage(pSock, peerCommand, rightNebIP, rightNebPort);
                    }
                }
				else if (peerPart[0] == "find-event"){
    			int eventID = stoi(peerPart[1]);
   				string senderName = peerPart[2];
    			string senderIP = peerPart[3];
                int senderPort = stoi(peerPart[4]);
                int visits = stoi(peerPart[5]);
                bool beenThere[20] = {};

                for(int i = 0; i < visits; i++){
                    beenThere[stoi(peerPart[6 + i])] = true;
                }

                beenThere[id] = true;

                int pos = eventID % s;
                int peerID = pos % ringSize;

                string idSeq = "";
            
                for(int i = 0; i < visits; i++){
                    idSeq = idSeq + peerPart[6 + i] + " ";
                }

                idSeq = idSeq + to_string(id);

                 if (peerID == id){
                    if (hashTable[pos] != ""){
                        if (stoi(hashTable[pos].substr(0, hashTable[pos].find(','))) == eventID){
                            peerMessage(pSock, "SUCCESS " + hashTable[pos] + "|" + idSeq, senderIP, senderPort);
                        }
                        else{
                            peerMessage(pSock, "FAILURE " + to_string(eventID), senderIP, senderPort);
                        }
                    }
                    else{
                        peerMessage(pSock, "FAILURE " + to_string(eventID), senderIP, senderPort);
                    }
                }
                else{
                    if (visits + 1 == ringSize){
                        peerMessage(pSock, "FAILURE " + to_string(eventID), senderIP, senderPort);
                    }
                    else{
                        int nextID = rand() % ringSize;

                        while(beenThere[nextID] == true){
                            nextID = rand() % ringSize;
                        }
            
                        string nextMess = "find-event " + to_string(eventID) + " " + senderName + " " + senderIP + " " + to_string(senderPort) + " " + to_string(visits + 1);

                        for(int i = 0; i < visits; i++){
                            nextMess = nextMess + " " + peerPart[6 + i];
                        }

                        nextMess = nextMess + " " + to_string(id);

                        peerMessage(pSock, nextMess, ips[nextID], pPorts[nextID]);
                    }
                }
            }
			else if(peerPart[0] == "teardown"){
    			for(int i = 0; i < 50000; i++){
        			hashTable[i] = "";
    			}

    			if(id == 0){
        			string completeMess = "teardown-complete " + myName;
        			sendto(sock, completeMess.c_str(), completeMess.size(), 0, (sockaddr *)&manAdd, sizeof(manAdd));

        			char mess[1024];
        			sockaddr_in replyAdd;
       				socklen_t replyAddSize = sizeof(replyAdd);
        			int messSize = recvfrom(sock, mess, 1023, 0, (sockaddr *)&replyAdd, &replyAddSize);
        			mess[messSize] = '\0';

        			cout << mess << endl;

        			id = -1;
        			ringSize = 0;
        			s = 0;
       				rightNebIP = "";
       				rightNebPort = 0;
    			}
    			else{
        			peerMessage(pSock, peerCommand, rightNebIP, rightNebPort);

        			id = -1;
        			ringSize = 0;
        			s = 0;
        			rightNebIP = "";
        			rightNebPort = 0;
    			}
			}
					
			else if (peerPart[0] == "FAILURE"){
   			int eventID = stoi(peerPart[1]);
    		cout << "Storm event " << eventID << " not found in the DHT." << endl;
			}
			else if (peerPart[0] == "SUCCESS"){
    			    int separationPoint = peerCommand.find('|');
    				string record = peerCommand.substr(8, separationPoint - 8);
    				string idSeq = peerCommand.substr(separationPoint + 1);
					string field[14];
    				int fieldNum = 0;

   					 for(int i = 0; i < record.length(); i++){
        				if(record[i] == ','){
           				 	fieldNum++;
       					 }
       					 else{
            				field[fieldNum] = field[fieldNum] + record[i];
       					 }
   				 }

    				cout << "event id: " << field[0] << endl;
    				cout << "state: " << field[1] << endl;
    				cout << "year: " << field[2] << endl;
   					cout << "month name: " << field[3] << endl;
   					cout << "event type: " << field[4] << endl;
    				cout << "cz type: " << field[5] << endl;
    				cout << "cz name: " << field[6] << endl;
    				cout << "injuries direct: " << field[7] << endl;
    				cout << "injuries indirect: " << field[8] << endl;
    				cout << "deaths direct: " << field[9] << endl;
    				cout << "deaths indirect: " << field[10] << endl;
    				cout << "damage property: " << field[11] << endl;
    				cout << "damage crops: " << field[12] << endl;
    				cout << "tor f scale: " << field[13] << endl;
    				cout << "id-seq: " << idSeq << endl;
			}
            
        }
		
    }

    return 0;
}
