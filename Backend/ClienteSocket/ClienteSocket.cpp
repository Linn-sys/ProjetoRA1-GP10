#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string> 

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup falhou: " << result << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in hint{};
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Erro ao conectar no servidor." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "[Cliente] Conectado ao servidor em 127.0.0.1:54000.\n";

    std::string msg;
    char buf[4096];

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, msg);

        if (msg == "sair") break;
        
        std::string jsonMsg = R"({"processo":1,"mensagem":")" + msg + "\"}";
        int sendResult = send(sock, jsonMsg.c_str(), (int)jsonMsg.size(), 0);

        if (sendResult != SOCKET_ERROR) {
            ZeroMemory(buf, 4096);
            int bytesReceived = recv(sock, buf, 4096, 0);
            if (bytesReceived > 0) {
                std::cout << "[Cliente] Servidor respondeu: " << std::string(buf, 0, bytesReceived) << std::endl;
            }
        }
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}
