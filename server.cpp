//Initialize winsock library
//Create the socket
// get ip and port
// bind the ip/port with the socket
// listen on the socket
//recv and send
//close the socket
//cleanup

#include <iostream>
#include <winsock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>
#include<algorithm>

using namespace std;

bool Initialize(){
    WSAData data;
    return WSAStartup (MAKEWORD(2,2), &data)==0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>&clients){
    cout<<"client connect "<<endl;
    char buffer[4096];
    while(1){
        int bytesrecvd = recv(clientSocket,buffer,sizeof(buffer),0);

        if(bytesrecvd<=0){
            cout<<"client disconnected"<<endl;
            break;
        }

        string message(buffer, bytesrecvd);
        cout<<"message from client: "<<message<<endl;

        for(auto client : clients){
            if(client!=clientSocket){
                send(client, message.c_str(), message.length(),0);
            }
            
        }
    } 

    auto it=find(clients.begin(),clients.end(),clientSocket);
    if(it!=clients.end()){
        clients.erase(it);
    }

    closesocket(clientSocket);
}

int main(){
    if(!Initialize()){
        cout<<"winsock initialization failed "<<endl;
    }

    SOCKET listenSocket = socket(AF_INET,SOCK_STREAM,0 );
    if (listenSocket==INVALID_SOCKET){
        cout<<"Socket creation failed"<<endl;
        return 1;
    }

    sockaddr_in serveraddr;
    int port = 12345;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    
    if(InetPton (AF_INET, _T("0.0.0.0"),&serveraddr.sin_addr) !=1){
        cout<<"Setting address structure failed "<<endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if(bind(listenSocket,reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr))== SOCKET_ERROR){
        cout<<"Binding failed "<<endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }


    if(listen(listenSocket,SOMAXCONN) == SOCKET_ERROR){
        cout<<"Listening failed "<<endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout<<"Server has started listening on port : "<<port<<endl;

    vector<SOCKET> clients;

    while(1){
        SOCKET clientSocket=accept(listenSocket,nullptr,nullptr);
        if(clientSocket == INVALID_SOCKET){
            cout<<"Invalid Client Socket "<<endl;
        }

        clients.push_back(clientSocket);
        thread t1(InteractWithClient, clientSocket,std::ref(clients));
        t1.detach();


    }  
    
    closesocket(listenSocket);

    WSACleanup();
    return 0;
}
