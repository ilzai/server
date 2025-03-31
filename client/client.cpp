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

using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        cerr << "Usage: ip_address port" << endl; exit(0); 
    }
    char *serverIp = argv[1]; int port = atoi(argv[2]); 
 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = inet_addr(serverIp);
    sendSockAddr.sin_port = htons(port);

    int clientSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(clientSd < 0){
        cerr << "Error creating socket" << endl;
        return -1;
    }

    //clear socket
    int reuse = 1;
    int result = setsockopt(clientSd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse));
    if (result < 0) {
        cerr << "ERROR SO_REUSEADDR:" << endl;
    }
    int status = connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
        return -1;
    }
    cout << "Connected to the server!" << endl;
    cout << "/************************/" << endl;
    cout << "To authorize user type 1" << endl;
    cout << "To register user type 2" << endl;
    cout << "To exit type 3" << endl;
    cout << "/************************/" << endl;
    while(1)
    {
        string login;
        string password;
        char serverResponse[50];
        int command = 0;
        int len = 0;

        //clearing input buffers
        memset(serverResponse, 0, sizeof(serverResponse));

        cout << "Enter command" << endl;
        cout << ">";
        do{
            scanf("%d", &command);
            char ch = getchar();
            if(command <= 0 || command > 3) cout << "Inviled command!" << endl;
        }while(command <= 0 || command > 3);

        send(clientSd, &command, 4, 0);
        if(command == 3){
            cout << "Client exit seccion..." << endl;
            return 0;
        }

        cout << "Enter login" << endl;
        cout << ">";
        do{
            getline(cin, login);
            if(login == "" || login.size() > 100) cout << "Invailed login format!" << endl;
        }while(login == "" || login.size() > 100);
        const char *cLogin = login.c_str();
        

        cout << "Enter password" << endl;
        cout << ">";
        do{
            getline(cin, password);
            if(password == "" || password.size() > 100) cout << "Invailed password format!" << endl;
        }while(password == "" || password.size() > 100);
        const char *cPassword = password.c_str();

        len = strlen(cLogin);
        send(clientSd, &len, 4, 0);
        send(clientSd, (char*)cLogin, len, 0);

        len = strlen(cPassword);
        send(clientSd, &len, 4, 0);
        send(clientSd, (char*)cPassword, len, 0);

        recv(clientSd, &len, 4, 0);
        recv(clientSd, (char*)serverResponse, len, 0);

        cout << "Server response: " << static_cast<string>((char*)serverResponse) << endl;
    }
    close(clientSd);
    return 0;    
}