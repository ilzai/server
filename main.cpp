#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <map>
#include <thread>
#include <vector>
#include <mutex>
#include "spdlog/spdlog.h"
#include <signal.h>

using namespace std;

class IClient{
public:
    virtual void loop(int) = 0;
    virtual ~IClient() = default;
};

class Client : public IClient{
public:
    Client(int newSd) : descriptor(newSd){
        cout << "Client constructor" << endl;
        clientThr = new thread(&Client::loop, this, newSd);
    }
    ~Client(){
        delete clientThr;
        close(descriptor);
    }
private:
    void loop(int descriptor) override{
        while(1){
            cout << "work" << " " << descriptor << endl;
            sleep(1);
        }
    }
    thread *clientThr = nullptr;
    int descriptor = -1;
};

class IServer{
public:
    virtual int getServerSd() = 0;
    virtual bool isCreated() = 0;
    virtual void acceptLoop() = 0;
    virtual ~IServer() = default;
};

class Server : public IServer{
public:
    Server(int port){
        //readBase();
        //int countOfClients = 0;

        spdlog::info<string>("Server started on port " + to_string(port) + "!");

        cout << "/************************/" << endl;
        cout << "To stop server type \"Ctrl + Z\"" << endl;
        cout << "/************************/" << endl;

        sockaddr_in servAddr;
        bzero((char*)&servAddr, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(port);
    
        serverSd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
        if(serverSd < 0)
        {
            spdlog::error<string>("Error establishing the server socket");
            exit(0);
        }else{
            spdlog::info<string>("Socket established");
        }

        int reuse = 1;
        int result = setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse));
        if (result < 0) {
            cerr << "ERROR SO_REUSEADDR:" << endl;
        }

        int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));
        if(bindStatus < 0)
        {
            spdlog::error<string>("Error binding socket to local address");
            exit(0);
        }else{
            spdlog::info<string>("Socket binded to local adress");
        }
        listen(serverSd, 50);
        created = true;
        spdlog::info<string>("Waiting for a client to connect...");
    }
    int getServerSd() override{
        return serverSd;
    }
    bool isCreated() override{
        return created;
    }
    void acceptLoop() override{
        int i = 0;
        while(1){
            sockaddr_in newSockAddr;
            socklen_t newSockAddrSize = sizeof(newSockAddr);

            int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
            if(newSd >= 0){
                spdlog::info<string>("Connected with client with descriptor " + to_string(newSd));
                clients.push_back(new Client(newSd));
            }
        }
    }
    ~Server(){
        cout << "Server destructor" << endl;
        for(IClient* client : clients){
            delete client;
        }
        close(serverSd);
    }
private:
    vector<IClient*> clients;
    int serverSd = -1;
    bool created = false;
};

IServer *serv;

void stopHandler(int param){
    cout << "Stopping server by SIGINT" << endl;
    delete serv;
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 2)
    {
        spdlog::error<string>("Enter port number");
        cerr << "Usage: port" << endl;
        exit(0);
    }
    int port = atoi(argv[1]);
    serv = new Server(port);
    signal(SIGINT, stopHandler);
    cout << "Created: " << serv->isCreated() << endl;
    cout << "Server descriptor: " << serv->getServerSd() << endl;
    if(serv->isCreated()){
        serv->acceptLoop();
    }
    return 0;
}