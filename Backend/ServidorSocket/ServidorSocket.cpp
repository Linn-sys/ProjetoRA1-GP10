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
        // Zera o buffer (Windows helper; alternativa portátil: std::fill_n(buf, 4096, 0))
        ZeroMemory(buf, 4096);

        // 'recv' é BLOQUEANTE por padrão: espera dados do cliente.
        // Retornos:
        //  >0 = bytes lidos; 0 = conexão fechada de forma ordenada; <0 = erro.
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived <= 0) break; // Sai do loop se cliente fechou ou houve erro

        // Constrói std::string com exatamente 'bytesReceived' bytes (pode conter '\0')
        std::string mensagem(buf, 0, bytesReceived);

        // ---------------- (1) BROADCAST DA MENSAGEM RECEBIDA ----------------
        std::cout << "[Servidor] Recebeu do cliente e retransmitindo: "
            << mensagem << std::endl;

        // Bloqueia o mutex para ler o vetor 'clientes' com segurança
        mtx.lock();
        for (SOCKET c : clientes) {
            // Envia a mensagem original para TODOS os clientes (inclui o remetente)
            // Obs.: não há tratamento de erro de 'send' aqui.
            send(c, mensagem.c_str(), (int)mensagem.size(), 0);
        }
        mtx.unlock();

        // ---------------- (2) MONTA RESPOSTA DO "PROCESSO 2" ----------------

        // Extrai o valor do campo "mensagem" de um JSON bem simples.
        // Observação IMPORTANTE:
        // - Esse parsing é frágil (não lida com espaços, escapes complexos,
        //   outras chaves, ordem diferente etc.). Ideal: usar biblioteca JSON.
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
        // Obs.: escapa só aspas; se houver \n, \r, \\, unicode, não cobre todos os casos.
        std::string mensagemServidorEscapada = mensagemServidor;
        size_t pos = 0;
        while ((pos = mensagemServidorEscapada.find('"', pos)) != std::string::npos) {
            mensagemServidorEscapada.replace(pos, 1, "\\\"");
            pos += 2; // avança depois da sequência de escape
        }

        // Monta o JSON da resposta (adiciona '\n' ao final; útil se cliente lê por linha)
        std::string respostaServidor =
            "{\"processo\":2,\"mensagem\":\"" + mensagemServidorEscapada + "\"}\n";

        // Envia a resposta do "processo 2" para TODOS os clientes (broadcast)
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao iniciar Winsock." << std::endl;
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

    // Associa socket ao endereço/porta
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
        sockaddr_in client;
        int clientSize = sizeof(client);

        // Aceita uma conexão. Retorna um novo socket para esse cliente.
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue; // ignora falhas e tenta de novo

        std::cout << "[Servidor] Cliente conectado!\n";

        // Adiciona o socket desse cliente ao vetor compartilhado (protegido por mutex)
        mtx.lock();
        clientes.push_back(clientSocket);
        mtx.unlock();

        // Cria a thread de atendimento e a "destaca" (detach)
        // Obs.: detach => sem join no final; a thread gerencia seu próprio ciclo de vida.
        std::thread t(handleClient, clientSocket);
        t.detach();
    }

    // Este ponto é inalcançável com o while(true) acima, mas deixado por clareza
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
