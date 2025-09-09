// Reduz include de headers da Win32 para acelerar compilação e evitar conflitos
#define WIN32_LEAN_AND_MEAN

// Cabeçalhos de sockets do Windows (Winsock 2)
#include <winsock2.h>

// Funções adicionais para manipulação de IPs e endereços
#include <ws2tcpip.h>

// STL e utilitários
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
// (Sugestão: #include <cstring> para strlen usado adiante)

// Vincula automaticamente a lib do Winsock 2 no MSVC (sem precisar mexer no projeto)
#pragma comment(lib, "ws2_32.lib")

// Lista global de sockets de clientes conectados (compartilhada entre threads)
std::vector<SOCKET> clientes;

// Mutex global para proteger acesso concorrente ao vetor 'clientes'
std::mutex mtx;

// Função executada por uma thread por cliente.
// Lê dados do socket, retransmite e envia a resposta "processo 2".
void handleClient(SOCKET clientSocket) {
    char buf[4096]; // Buffer de recepção (4 KB)
    while (true) {
        // Zera o buffer 
        ZeroMemory(buf, 4096); // Preenche o buffer com zeros

        // A função 'recv' da API Winsock bloqueia até receber os dados de um cliente
        //  >0 = bytes lidos; 0 = conexão fechada de forma ordenada; <0 = erro.
        int bytesReceived = recv(clientSocket, buf, 4096, 0); // Utiliza o socket do cliente, o buffer e o tamanho máximo
        if (bytesReceived <= 0) break; // Sai do loop se cliente fechou ou houve erro

        // Transcreve os bytes recebidos em uma mensagem string
        std::string mensagem(buf, 0, bytesReceived);

        // ---------------- (1) BROADCAST DA MENSAGEM RECEBIDA ----------------
        std::cout << "[Servidor] Recebeu do cliente e retransmitindo: "
            << mensagem << std::endl;

        // Bloqueia o mutex para ler o vetor 'clientes' 
        mtx.lock(); 
        for (SOCKET c : clientes) {
            // Envia a mensagem original para todos os clientes. Utiliza o iterador de clietes, a mensagem e o tamanho da mensagem 
            send(c, mensagem.c_str(), (int)mensagem.size(), 0); 
        }
        mtx.unlock(); // Desbloqueia o Mutex

        // ---------------- (2) MONTA RESPOSTA DO "Servidor" ----------------

        // Extrai o valor do campo "mensagem" de um JSON bem simples.
        std::string textoCliente;
        size_t posMensagem = mensagem.find("\"mensagem\":\"");
        if (posMensagem != std::string::npos) {
            // strlen do literal para encontrar o início do conteúdo do campo
            size_t startPos = posMensagem + strlen("\"mensagem\":\"");
            size_t endPos = mensagem.find("\"", startPos);
            if (endPos != std::string::npos) {
                textoCliente = mensagem.substr(startPos, endPos - startPos);
            }
        }

        // Constrói um texto de resposta do servidor concatenando o que veio do cliente
        std::string mensagemServidor =
            " Recebeu e processou a seguinte mensagem: " + textoCliente;

        // Escapa aspas duplas (") no conteúdo para gerar JSON válido
        std::string mensagemServidorEscapada = mensagemServidor;
        size_t pos = 0;
        while ((pos = mensagemServidorEscapada.find('"', pos)) != std::string::npos) {
            mensagemServidorEscapada.replace(pos, 1, "\\\"");
            pos += 2; // avança depois da sequência de escape
        }

        // Monta o JSON da resposta (adiciona '\n' ao final)
        std::string respostaServidor =
            "{\"processo\":2,\"mensagem\":\"" + mensagemServidorEscapada + "\"}\n";

        // Envia a resposta do "servidor" para TODOS os clientes (broadcast)
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
    // Struct com dados da implementação Winsock (API de sockets do Windows) carregada
    WSADATA wsaData;

    // Carrega e inicializa a DLL do Winsock com versão 2.2. Necessário passar um ponteiro para a Struck WSADATA
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) { // Se falhar, retorna um código de erro
        std::cerr << "WSAStartup falhou: " << result << std::endl;
        return 1;
    }

    // Cria um socket TCP (SOCK_STREAM) IPv4 (AF_INET). Protocolo = 0 -> automático (TCP).
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Armazena o socket na variável serverSocket
    if (serverSocket == INVALID_SOCKET) { // Se retornar erro, mostra mensagem e limpa a Winsock
        std::cerr << "Erro ao criar socket. Código: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Preenche endereço/porta do servidor
    sockaddr_in serverHint{}; // sockaddr_in é uma estrutura para endereços IPv4. {} zera todos os campos
    serverHint.sin_family = AF_INET; // Família de endereço IPv4 (AF_INET)
    serverHint.sin_port = htons(54000); // Define a porta (54000). htons() converte para o formato da rede
    inet_pton(AF_INET, "127.0.0.1", &serverHint.sin_addr); // inet_pton() converte o formato do IP em string para o formato da rede, já colocando na struct

    // Associa socket ao endereço/porta do servidor
    if (bind(serverSocket, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) { // bind() relaciona o socket criado com o ponteiro do endereço de serverHint feito o cast com o tipo sockaddr
        // Em caso de erro, encerra o socket do servidor e limpa a Winsock
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Coloca o socket em modo "passivo" para aceitar conexões; 
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) { // SOMAXCONN determina o tamanho da fila de conexões pendentes, sendo nesse caso, o máximo do SO
        // Em caso de erro, encerra o socket do servidor e limpa a Winsock
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[Servidor] Aguardando conexões em 127.0.0.1:54000...\n";

    // Loop principal de aceitação: bloqueia até aparecer um cliente
    while (true) {
        sockaddr_in client; // Cria uma struct de enderaçamento do cliente para receber os dados da conexão
        int clientSize = sizeof(client); // Armazena o tamanho da struct do endereço do cliente

        // Função que aceita uma conexão do cliente. Retorna um novo socket para esse cliente no servidor. Utiliza o socket do servidor, o ponteiro do endereço do cliente do tipo sockaddr e o tamanho da struck do endereço do cliente 
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue; // Caso ocorra alguma falha, reinicia o loop indo para a próxima iteração

        std::cout << "[Servidor] Cliente conectado!\n";

        // Adiciona o socket desse cliente ao vetor compartilhado (protegido por mutex para evitar que threads mexam na lista de clientes ao mesmo tempo)
        mtx.lock(); // Bloqueia o Mutex
        clientes.push_back(clientSocket); // Adiciona o socket no vetor de sockets de clientes
        mtx.unlock(); // Libera o Mutex

        // Cria a thread de atendimento e a "destaca" (detach)
        // Obs.: detach => sem join no final; a thread gerencia seu próprio ciclo de vida.
        std::thread t(handleClient, clientSocket); // Cria uma therad utilizando a função handleClient(), enviando o handle do socket do cliente
        t.detach(); // Roda a thread em "segundo plano", e ao finalizar, 
    }

    // Fecha o socket do servidor e limpa a Winsock
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
