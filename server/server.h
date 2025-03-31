#pragma once
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

#define LOGING 1
#define REGISTER 2

#define WRONG_PASSWORD 1
#define USER_NOT_EXISTS 2
#define USER_EXISTS 3

#define USER_ALREADY_EXISTS 4
#define USER_REGISTERED 5

extern map<string, string> base;
extern mutex mtxBase;

#if TESTING
#define PRIVATE_TESTABLE public
#else
#define PRIVATE_TESTABLE private
#endif

class IClient{
    public:
        virtual int login(string, string) = 0;
        virtual int reg(string, string) = 0;
        virtual void loop(int) = 0;
        virtual ~IClient() = default;
    };
    
    class Client : public IClient{
    public:
        Client(int newSd);
        ~Client();
    private:
    PRIVATE_TESTABLE:
        int login(string log, string password) override;
        int reg(string log, string password) override;
        void loop(int descriptor) override;
        thread *clientThr = nullptr;
        int descriptor = -1;
    };
    
    class IServer{
    public:
        virtual int getServerSd() = 0;
        virtual bool isCreated() = 0;
        virtual void acceptLoop() = 0;
        virtual bool readBase() = 0;
        virtual bool writeBase() = 0;
        virtual ~IServer() = default;
    };
    
    class Server : public IServer{
    public:
        Server(int port);
        int getServerSd() override;
        bool isCreated() override;
        void acceptLoop() override;
        ~Server();
    private:
    PRIVATE_TESTABLE:
        bool readBase() override;
        bool writeBase() override;
        vector<IClient*> clients;
        int serverSd = -1;
        bool created = false;
    };