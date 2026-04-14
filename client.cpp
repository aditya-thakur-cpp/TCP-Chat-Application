//initialize WinSock
//create socket
//connect to the server
//send/recv
//close the socket

#include<iostream>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<thread>

using namespace std;

bool Initialize(){
    WSADATA data;
    return WSAStartup ( MAKEWORD(2,2), &data)==0;
}

void SendMsg(SOCKET s){
    cout<<"Enter your chat name : "<<endl;
    string name;

    getline(cin, name);
    string message;

    while(1){
        getline(cin, message);
        string msg = name + " : " + message;
        int bytesent = send(s, msg.c_str(),msg.length(),0);
        if(bytesent == SOCKET_ERROR){
            cout<<"error sending message "<<endl;
            break;
        }
        if(message =="quit"){
            cout<<"stopping the application "<<endl;
            break;
        }
    }
    closesocket(s);
    WSACleanup();
}

void ReceiveMsg(SOCKET s){
    while(1){
        char buffer[4096];
        int recvlength;
        string msg= "";
        recvlength = recv(s, buffer, sizeof(buffer),0);
        if(recvlength <=0){
            cout<<"disconnected from the server "<<endl;
            break;
        }
        else{
            msg = string(buffer, recvlength);
            cout<<msg<<endl;
        }

    }
}

int main(){
    cout<<"Client program started "<<endl;

    if(!Initialize()){
        cout<<"Initialize winsock failed "<<endl;

        return 1;
    }

    SOCKET s=socket(AF_INET, SOCK_STREAM, 0); 
    if( s == INVALID_SOCKET){
        cout<<"Invalid scoket created "<<endl;
        return 1;
    }   

    int port= 12345;
    string serveraddress = "192.168.1.9";
    sockaddr_in serveraddr;
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    inet_pton(AF_INET,serveraddress.c_str(), &(serveraddr.sin_addr));

   

    if(connect(s, reinterpret_cast<sockaddr*>(&serveraddr),sizeof(serveraddr))==SOCKET_ERROR){
        cout<<"not able to connect to server "<<endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout<<"Successfully connected to server "<<endl;

    thread senderthread(SendMsg , s);
    thread receiver(ReceiveMsg , s);
    senderthread.join();
    receiver.join();


    // string message = "hello";
    // int bytesent;
    // if(bytesent=send(s,message.c_str(),message.length(),0)==SOCKET_ERROR){
    //     cout<<"send failed"<<endl;
    //     closesocket(s);
    //     WSACleanup();
    //     return 1;
    // }

    // WSACleanup();
    return 0;
}