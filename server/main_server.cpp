#include "server.h"

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