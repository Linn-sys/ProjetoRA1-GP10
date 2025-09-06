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

        // Limpa a string de caracteres de nova linha ou retorno de carro
        mensagem.erase(std::remove(mensagem.begin(), mensagem.end(), '\n'), mensagem.end());
        mensagem.erase(std::remove(mensagem.begin(), mensagem.end(), '\r'), mensagem.end());

        std::cout << "[Servidor] Recebeu do cliente: " << mensagem << std::endl;

        // Escapa as aspas duplas na mensagem
        std::string mensagemEscapada = mensagem;
        size_t pos = 0;
        while ((pos = mensagemEscapada.find('"', pos)) != std::string::npos) {
            mensagemEscapada.replace(pos, 1, "\\\"");
            pos += 2;
        }

        // ENVIA A MENSAGEM DO CLIENTE PARA O NODE COMO PROCESSO 1
        std::string respostaCliente = R"({"processo":1,"mensagem":")" + mensagemEscapada + "\"}\n";

        mtx.lock();
        for (SOCKET c : clientes) {
            send(c, respostaCliente.c_str(), (int)respostaCliente.size(), 0);
        }
        mtx.unlock();

        // ENVIA UMA MENSAGEM DO SERVIDOR PARA O NODE COMO PROCESSO 2
        std::string respostaServidor = R"({"processo":2,"mensagem":"Servidor recebeu e processou a mensagem.)" + mensagemEscapada + "\"}\n";

        mtx.lock();
        for (SOCKET c : clientes) {
            send(c, respostaServidor.c_str(), (int)respostaServidor.size(), 0);
            send(c, "\n", 1, 0);
        }
        mtx.unlock();
    }
    mtx.lock();
    clientes.erase(std::remove(clientes.begin(), clientes.end(), clientSocket), clientes.end());
    mtx.unlock();
}
//void enviarServidor(std::string mensagem) {
//    for (size_t pos = 0; (pos = mensagem.find('"', pos)) != std::string::npos; pos += 2) {
//        mensagem.replace(pos, 1, "\\\"");
//    }
//    //for (auto& ch : mensagem) if (ch == '\"') ch = '\'';
//    std::string respostaServidor = R"({"processo":2,"mensagem":")" + mensagem + "\"}\n";
//
//    mtx.lock();
//    for (SOCKET c : clientes) {
//        send(c, respostaServidor.c_str(), (int)respostaServidor.size(), 0);
//    }
//    mtx.unlock();
//}


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

    //// no main(), depois do while(accept)
    //std::thread tServidor([]() {
    //    std::string msgServidor;
    //    while (true) {
    //        std::getline(std::cin, msgServidor);
    //        if (msgServidor == "sair") break;
    //        enviarServidor(msgServidor);
    //    }
    //    });
    //tServidor.detach();


    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
