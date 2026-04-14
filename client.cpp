#include "common.h"

atomic<bool> shouldExit(false);

void SendMsg(SOCKET s, const string& name) {
    string message;
    while (!shouldExit) {
        getline(cin, message);

        if (message == "quit") {
            shouldExit = true;
            cout << "Disconnecting..." << endl;
            break;
        }

        string msg = name + " : " + message;
        if (!sendFramed(s, msg)) {
            cerr << "Send failed. Error: " << WSAGetLastError() << endl;
            shouldExit = true;
            break;
        }
    }
}


void ReceiveMsg(SOCKET s) {
    string message;
    while (!shouldExit) {
        if (!recvFramed(s, message)) {
            if (!shouldExit)
                cout << "Disconnected from server." << endl;
            shouldExit = true;
            break;
        }
        cout << message << endl;
    }
}


int main(int argc, char* argv[]) {
    string serverIP = (argc > 1) ? argv[1] : "127.0.0.1";
    int    port     = (argc > 2) ? stoi(argv[2]) : PORT;

    cout << "Connecting to " << serverIP << ":" << port << " ..." << endl;

    if (!Initialize()) {
        cerr << "WinSock init failed. Error: " << WSAGetLastError() << endl;
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port   = htons(port);
    if (inet_pton(AF_INET, serverIP.c_str(), &serveraddr.sin_addr) != 1) {
        cerr << "Invalid server IP." << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cerr << "Connection failed. Error: " << WSAGetLastError() << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Connected! Enter your chat name: ";
    string name;
    getline(cin, name);

    
    if (!sendFramed(s, name)) {
        cerr << "Failed to send name." << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Welcome, " << name << "! Type 'quit' to exit." << endl;

    thread senderThread(SendMsg, s, name);
    thread receiverThread(ReceiveMsg, s);

    senderThread.join();
    receiverThread.join();

    
    closesocket(s);
    WSACleanup();
    return 0;
}