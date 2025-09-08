#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clientes;
std::mutex mtx;

void handleClient(SOCKET clientSocket) {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived <= 0) break;

        std::string mensagem(buf, 0, bytesReceived);

        // 1. ENVIA A MENSAGEM DO CLIENTE PARA O NODE (PROCESSO 1)
        std::cout << "[Servidor] Recebeu do cliente e retransmitindo: " << mensagem << std::endl;
        mtx.lock();
        for (SOCKET c : clientes) {
            send(c, mensagem.c_str(), (int)mensagem.size(), 0);
        }
        mtx.unlock();

        // 2. ENVIA A MENSAGEM DE RESPOSTA DO SERVIDOR PARA O NODE (PROCESSO 2)

        // Extrai a mensagem de texto do JSON do cliente
        std::string textoCliente;
        size_t posMensagem = mensagem.find("\"mensagem\":\"");
        if (posMensagem != std::string::npos) {
            size_t startPos = posMensagem + strlen("\"mensagem\":\"");
            size_t endPos = mensagem.find("\"", startPos);
            if (endPos != std::string::npos) {
                textoCliente = mensagem.substr(startPos, endPos - startPos);
            }
        }

        // Constrói a mensagem de resposta do servidor, concatenando o texto extraído
        std::string mensagemServidor = " Recebeu e processou a seguinte mensagem: " + textoCliente;

        // Escapa as aspas duplas na nova mensagem do servidor
        std::string mensagemServidorEscapada = mensagemServidor;
        size_t pos = 0;
        while ((pos = mensagemServidorEscapada.find('"', pos)) != std::string::npos) {
            mensagemServidorEscapada.replace(pos, 1, "\\\"");
            pos += 2;
        }

        // Monta o JSON para a resposta do servidor.
        std::string respostaServidor = "{\"processo\":2,\"mensagem\":\"" + mensagemServidorEscapada + "\"}\n";

        mtx.lock();
        for (SOCKET c : clientes) {
            send(c, respostaServidor.c_str(), (int)respostaServidor.size(), 0);
        }
        mtx.unlock();
    }
    mtx.lock();
    clientes.erase(std::remove(clientes.begin(), clientes.end(), clientSocket), clientes.end());
    mtx.unlock();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) { WSACleanup(); return 1; }

    sockaddr_in serverHint{};
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverHint.sin_addr);

    if (bind(serverSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) {
        closesocket(serverSocket); WSACleanup(); return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket); WSACleanup(); return 1;
    }

    std::cout << "[Servidor] Aguardando conexões em 127.0.0.1:54000...\n";

    while (true) {
        sockaddr_in client;
        int clientSize = sizeof(client);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue;

        std::cout << "[Servidor] Cliente conectado!\n";

        mtx.lock();
        clientes.push_back(clientSocket);
        mtx.unlock();

        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
