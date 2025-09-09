// Evita que o Windows inclua cabeçalhos desnecessários
#define WIN32_LEAN_AND_MEAN 

// Biblioteca principal do Winsock 2 (sockets no Windows)
#include <winsock2.h>  

// Funções adicionais para manipulação de IPs e endereços
#include <ws2tcpip.h>  

// Bibliotecas padrão do C++ para entrada/saída e strings
#include <iostream>
#include <string> 
#include <vector>

// Vincula automaticamente a aplicação com a biblioteca do Winsock
#pragma comment(lib, "ws2_32.lib")

int main() {
    // Estrutura que receberá informações sobre a implementação do Winsock
    WSADATA wsaData;

    // Inicializa o Winsock (versão 2.2)
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) { // Se falhar, retorna um código de erro
        std::cerr << "WSAStartup falhou: " << result << std::endl;
        return 1;
    }

    // Cria um socket TCP (SOCK_STREAM) sobre IPv4 (AF_INET)
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) { // Em caso de falha
        std::cerr << "Erro ao criar socket." << std::endl;
        WSACleanup(); // Libera recursos do Winsock
        return 1;
    }

    // Estrutura que guarda o endereço do servidor
    sockaddr_in hint{};
    hint.sin_family = AF_INET;              // IPv4
    hint.sin_port = htons(54000);           // Porta do servidor (54000), convertida para big-endian
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr); // Converte o IP string → formato binário

    // Solicita a conexão com o servidor no IP/porta definidos
    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Erro ao conectar no servidor." << std::endl;
        closesocket(sock);  // Fecha o socket em caso de falha
        WSACleanup();       // Limpa o Winsock
        return 1;
    }

    std::cout << "[Cliente] Conectado ao servidor em 127.0.0.1:54000.\n";

    // Buffer para armazenar mensagens recebidas do servidor
    char buf[4096];
    // String para armazenar a entrada do usuário
    std::string msg;

    // Loop principal: lê mensagens digitadas no console
    while (std::getline(std::cin, msg)) {
        if (msg == "sair") break; // Encerra se o usuário digitar "sair"

        // ---- Escapar aspas duplas para JSON válido ----
        std::string mensagemEscapada = msg;
        size_t pos = 0;
        while ((pos = mensagemEscapada.find('"', pos)) != std::string::npos) {
            // Substitui " por \" no texto
            mensagemEscapada.replace(pos, 1, "\\\"");
            pos += 2; // Avança para não cair em loop infinito
        }

        // ---- Monta o JSON para enviar ao servidor ----
        // Exemplo: {"processo":1,"mensagem":"Olá mundo"}
        std::string jsonMsg = R"({"processo":1,"mensagem":")" + mensagemEscapada + "\"}\n";

        // ---- Envia o JSON pelo socket ----
        int sendResult = send(sock, jsonMsg.c_str(), (int)jsonMsg.size(), 0);

        // ---- Se envio foi bem-sucedido, espera as respostas ----
        if (sendResult != SOCKET_ERROR) {
            // Servidor deve mandar duas respostas (processo 1 e 2)

            // Primeira resposta do servidor (retransmissão)
            int bytesReceived1 = recv(sock, buf, 4096, 0);
            if (bytesReceived1 > 0) {
                // Neste código original não imprime nada,
                // apenas lê para consumir do buffer
                // Poderíamos imprimir aqui: std::cout << std::string(buf, bytesReceived1);
            }

            // Segunda resposta do servidor (processada)
            int bytesReceived2 = recv(sock, buf, 4096, 0);
            if (bytesReceived2 > 0) {
                // Também lida mas ignorada
            }
        }
    }

    // Fecha o socket
    closesocket(sock);

    // Libera os recursos do Winsock
    WSACleanup();

    return 0; // Finaliza o programa
}
