#include "common.h"
#include <tchar.h>

// ─── Shared State ─────────────────────────────────────────────
vector<SOCKET> clients;
mutex          clientsMutex;
atomic<int>    connectedCount(0);

// ─── Broadcast to all clients except sender ───────────────────
void broadcast(const string& message, SOCKET senderSocket) {
    lock_guard<mutex> lock(clientsMutex);
    for (auto client : clients) {
        if (client != senderSocket) {
            sendFramed(client, message);
        }
    }
}

// ─── Per-Client Thread ────────────────────────────────────────
void InteractWithClient(SOCKET clientSocket, string clientName) {
    string joinMsg = getTimestamp() + SERVER_TAG + " " + clientName + " has joined the chat.";
    cout << joinMsg << endl;
    broadcast(joinMsg, clientSocket);

    string message;
    while (true) {
        if (!recvFramed(clientSocket, message)) {
            break;
        }
        string stamped = getTimestamp() + message;
        cout << stamped << endl;
        broadcast(stamped, clientSocket);
    }

    // Remove from clients list
    {
        lock_guard<mutex> lock(clientsMutex);
        auto it = find(clients.begin(), clients.end(), clientSocket);
        if (it != clients.end()) clients.erase(it);
    }

    connectedCount--;
    string leaveMsg = getTimestamp() + SERVER_TAG + " " + clientName
                    + " has left. Online: " + to_string(connectedCount);
    cout << leaveMsg << endl;
    broadcast(leaveMsg, clientSocket);

    closesocket(clientSocket);
}

// ─── Main ─────────────────────────────────────────────────────
int main() {
    if (!Initialize()) {
        cerr << "WinSock init failed. Error: " << WSAGetLastError() << endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port   = htons(PORT);
    if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
        cerr << "Address setup failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cerr << "Bind failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed. Error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server listening on port " << PORT << " ..." << endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "accept() failed. Error: " << WSAGetLastError() << endl;
            continue;
        }

        // First message from client is always their name
        string clientName;
        if (!recvFramed(clientSocket, clientName)) {
            closesocket(clientSocket);
            continue;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        connectedCount++;
        cout << "New connection [" << clientName << "]. Online: " << connectedCount << endl;

        thread t(InteractWithClient, clientSocket, clientName);
        t.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}