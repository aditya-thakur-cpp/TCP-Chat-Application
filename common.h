#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>

using namespace std;

// ─── Constants ───────────────────────────────────────────────
const int    PORT        = 12345;
const int    BUFFER_SIZE = 4096;
const string SERVER_TAG  = "[Server]";

// ─── WinSock Init ─────────────────────────────────────────────
inline bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// ─── Timestamp ───────────────────────────────────────────────
inline string getTimestamp() {
    time_t now = time(0);
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
    return string("[") + buf + "] ";
}

// ─── sendAll — handles partial send() ────────────────────────
inline int sendAll(SOCKET s, const char* buf, int len) {
    int total = 0;
    while (total < len) {
        int sent = send(s, buf + total, len - total, 0);
        if (sent == SOCKET_ERROR) return SOCKET_ERROR;
        total += sent;
    }
    return total;
}

// ─── Message Framing — 4-byte length prefix ──────────────────
inline bool sendFramed(SOCKET s, const string& msg) {
    uint32_t msgLen = htonl((uint32_t)msg.size());
    if (sendAll(s, (char*)&msgLen, 4) == SOCKET_ERROR) return false;
    if (sendAll(s, msg.c_str(), (int)msg.size()) == SOCKET_ERROR) return false;
    return true;
}

inline bool recvFramed(SOCKET s, string& out) {
    char lenBuf[4];
    int received = 0;

    // Read exactly 4 bytes for the length header
    while (received < 4) {
        int r = recv(s, lenBuf + received, 4 - received, 0);
        if (r <= 0) return false;
        received += r;
    }

    uint32_t msgLen = 0;
    memcpy(&msgLen, lenBuf, 4);
    msgLen = ntohl(msgLen);

    if (msgLen == 0 || msgLen > (uint32_t)BUFFER_SIZE) return false;

    // Read exactly msgLen bytes for the payload
    string buf(msgLen, '\0');
    received = 0;
    while (received < (int)msgLen) {
        int r = recv(s, &buf[received], msgLen - received, 0);
        if (r <= 0) return false;
        received += r;
    }
    out = buf;
    return true;
}