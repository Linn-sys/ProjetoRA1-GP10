#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup falhou: " << result << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverHint{};
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(54000); // porta
    inet_pton(AF_INET, "127.0.0.1", &serverHint.sin_addr);

    if (bind(serverSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        std::cerr << "Erro no bind." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Erro no listen." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[Servidor] Aguardando conexões em 127.0.0.1:54000...\n";

    sockaddr_in client;
    int clientSize = sizeof(client);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Erro no accept." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    ZeroMemory(host, NI_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        std::cout << "[Servidor] Conectado com " << host << " na porta " << service << std::endl;
    }

    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived <= 0) break;

        std::cout << "[Servidor] Recebeu: " << std::string(buf, 0, bytesReceived) << std::endl;

        std::string resposta = "Eco do servidor: " + std::string(buf, 0, bytesReceived);
        send(clientSocket, resposta.c_str(), (int)resposta.size(), 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
