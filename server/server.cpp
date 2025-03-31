#include "server.h"

map<string, string> base;
mutex mtxBase;

Client::Client(int newSd) : descriptor(newSd){
    cout << "Client constructor" << endl;
    clientThr = new thread(&Client::loop, this, newSd);
}

Client::~Client(){
    close(descriptor);
    clientThr->~thread();
    delete clientThr;
}

int Client::login(string log, string password){
    map<string, string>::iterator search = base.find(log);
    if(search != base.end()){
        if(search->second == password){
            return USER_EXISTS;
        }else{
            return WRONG_PASSWORD;
        }
    }else{
        return USER_NOT_EXISTS;
    }
}

int Client::reg(string log, string password){
    int ret = login(log, password);
    if(ret == USER_EXISTS || ret == WRONG_PASSWORD){
        return USER_ALREADY_EXISTS;
    }else{
        base[log] = password;
        return USER_REGISTERED;
    }
}

void Client::loop(int descriptor){
    while(1){
        int bytesRead = 0; 
        int bytesWritten = 0;
        char log[100];
        char password[100];
        int command = 0;
        int len = 0;

        //clearing input buffers
        memset(log, 0, sizeof(log));
        memset(password, 0, sizeof(password));

        spdlog::info<string>("Server waiting for client response");
        cout << "Awaiting client response..." << endl;

        bytesRead += recv(descriptor, &command, 4, 0);
        if(bytesRead == 0){
            spdlog::info<string>("Client with descriptor " + to_string(descriptor) + " leaved session");
            cout << "client with sd: " << descriptor << " leaved session" << endl;
            break;
        }
        spdlog::info<string>("Client " + to_string(descriptor) + " sended command: " + to_string(command));
        cout << "sd " << descriptor << "sended command: " << command << endl;
        if(command == 3){
            spdlog::info<string>("Client " + to_string(descriptor) + " leaved session");
            cout << "client with sd: " << descriptor << " leaved session" << endl;
            break;
        }

        //receive size of message
        bytesRead += recv(descriptor, &len, 4, 0);
        if(bytesRead == 0){
            spdlog::info<string>("Client with descriptor " + to_string(descriptor) + " leaved session");
            cout << "client with sd: " << descriptor << " leaved session" << endl;
            break;
        }
        bytesRead += recv(descriptor, (char*)log, len, 0);
        spdlog::info<string>("Client " + to_string(descriptor) + " sended login: " + string(log));
        cout << "sd " << descriptor << "sended login: " << static_cast<string>(log) << endl;

        bytesRead += recv(descriptor, &len, 4, 0);
        if(bytesRead == 0){
            spdlog::info<string>("Client with descriptor " + to_string(descriptor) + " leaved session");
            cout << "client with sd: " << descriptor << " leaved session" << endl;
            break;
        }
        bytesRead += recv(descriptor, (char*)password, len, 0);
        spdlog::info<string>("Client " + to_string(descriptor) + " sended password: " + string(password));
        cout << "sd " << descriptor << "sended password: " << static_cast<string>(password) << endl;
        
        char retMessage[50];
        mtxBase.lock();
        switch(command){
            case LOGING:
                switch(login(log, password)){
                    case USER_EXISTS:
                        spdlog::info<string>("User already exists");
                        strcpy(retMessage, "User already exists");
                        break;
                    case USER_NOT_EXISTS:
                        spdlog::error<string>("User not exists");
                        strcpy(retMessage, "User not exists");
                        break;
                    case WRONG_PASSWORD:
                        spdlog::error<string>("Wrong password");
                        strcpy(retMessage, "Wrong password");
                        break;
                    default:
                        spdlog::error<string>("Wrong return code from loging");
                }
                break;
            case REGISTER:
                switch(reg(log, password)){
                    case USER_REGISTERED:
                        spdlog::info<string>("User succesfully added");
                        strcpy(retMessage, "User succesfully added");
                        break;
                    case USER_ALREADY_EXISTS:
                        spdlog::info<string>("User already exists");
                        strcpy(retMessage, "User already exists");
                        break;
                    default:
                        spdlog::error<string>("Wrong return code from registration");
                }
                break;
            default:
                cout << "Error Command to server" << endl;
        }
        mtxBase.unlock();
        len = strlen(retMessage);
        send(descriptor, &len, 4, 0);
        send(descriptor, (char*)retMessage, strlen(retMessage), 0);
    }
}

Server::Server(int port){
    readBase();
    //int countOfClients = 0;

    spdlog::info<string>("Server started on port " + to_string(port) + "!");

    cout << "/************************/" << endl;
    cout << "To stop server type \"Ctrl + C\"" << endl;
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

int Server::getServerSd(){
    return serverSd;
}

bool Server::isCreated(){
    return created;
}

void Server::acceptLoop(){
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

Server::~Server(){
    writeBase();
    close(serverSd);
    for(IClient* client : clients){
        delete client;
    }
}

bool Server::readBase(){
    ifstream bs;
    bs.open("../base.csv");
    bool ret;
    if(bs){
        string log, password;
        while(bs >> log >> password){
            base[log] = password;
        }
        ret = true;
        bs.close();
    }else{
        cout << "File opening error on reading, create file..." << endl;
        ret = false;
    }
    return ret;
}

bool Server::writeBase(){
    ofstream bs;
    bs.open("../base.csv");
    bool ret;
    if(bs){
        cout << "File writing..." << endl;
        for(auto elem : base){
            bs << elem.first << " " << elem.second << endl;
        }
        ret = true;
        bs.close();
    }else{
        cout << "File opening error on writing" << endl;
        ret = false;
    }
    return ret;
}