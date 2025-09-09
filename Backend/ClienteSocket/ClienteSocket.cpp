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
    // Struct com dados da implementação Winsock (API de sockets do Windows) carregada
    WSADATA wsaData;

    // Carrega e inicializa a DLL do Winsock com versão 2.2. Necessário passar um ponteiro para a Struck WSADATA
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) { // Se falhar, retorna um código de erro
        std::cerr << "WSAStartup falhou: " << result << std::endl;
        return 1;
    }

    // Cria um socket TCP (SOCK_STREAM) sobre IPv4 (AF_INET). Protocolo = 0 -> automático (TCP).
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) { // Em caso de falha
        std::cerr << "Erro ao criar socket. Código: " << WSAGetLastError() << std::endl;
        WSACleanup(); // Libera recursos do Winsock
        return 1;
    }

    // Preenche endereço/porta do servidor
    sockaddr_in hint{};                     // sockaddr_in é uma estrutura para endereços IPv4. {} zera todos os campos
    hint.sin_family = AF_INET;              // Família de endereço IPv4 (AF_INET)
    hint.sin_port = htons(54000);           // Define a porta (54000). htons() converte para o formato da rede
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr); // inet_pton() converte o formato do IP em string para o formato da rede, já colocando na struct

    // Função da API Winsock que solicita a conexão do cliente com o servidor. Utiliza o socket do cliente, o ponteiro do endereço do tipo sockaddr e o tamanho da struct do endereço hint
    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) { // Caso ocorra algum problema, informa o usuário, fecha o socket do cliente e limpa a Winsock
        std::cerr << "Erro ao conectar no servidor." << std::endl; 
        closesocket(sock);  
        WSACleanup();      
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
        std::string mensagemEscapada = msg; // Cria uma cópia da mensagem do usuário
        size_t pos = 0; 
        while ((pos = mensagemEscapada.find('"', pos)) != std::string::npos) { // Procura a primeira ocorrência de aspas duplas a partir da posição pos -> 0
            // Substitui " por \" no texto
            mensagemEscapada.replace(pos, 1, "\\\""); 
            pos += 2; // Avança para não cair em loop infinito
        }

        // ---- Monta o JSON para enviar ao servidor ----
        // Exemplo: {"processo":1,"mensagem":"Olá mundo"}
        std::string jsonMsg = R"({"processo":1,"mensagem":")" + mensagemEscapada + "\"}\n";

        // ---- Envia o JSON pelo socket ----
        int sendResult = send(sock, jsonMsg.c_str(), (int)jsonMsg.size(), 0); // Envia a mensagem em formato JSON. Utiliza o socket do cliente, a mensagem e o tamanho da mensagem 

        // ---- Se envio foi bem-sucedido, espera as respostas ----
        if (sendResult != SOCKET_ERROR) {
            // Servidor deve mandar duas respostas (processo 1 e 2)

            // Primeira resposta do servidor (retransmissão)
            int bytesReceived1 = recv(sock, buf, 4096, 0);
            if (bytesReceived1 > 0) {
                // Não fazemos nada, apenas garantimos que os dados são lidos e o buffer é limpado
            }

            // Segunda resposta do servidor (processada)
            int bytesReceived2 = recv(sock, buf, 4096, 0);
            if (bytesReceived2 > 0) {
                // e a segunda.
            }
        }
    }

    // Fecha o socket
    closesocket(sock);

    // Libera os recursos do Winsock
    WSACleanup();

    return 0; // Finaliza o programa
}
