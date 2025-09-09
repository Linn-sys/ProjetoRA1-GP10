//#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#include <windows.h>            // Declarações das APIs do Windows: CreateFileMapping, MapViewOfFile, CreateMutex, WaitForSingleObject etc.
#include <iostream>             // std::cout, std::cin, std::cerr para I/O no console
#include <string>               // std::string para manipular texto
#include <thread>               // std::thread para criar a thread de leitura
#include <chrono>               // std::chrono para sleep no loop da thread
#include <cstdint>              // tipos de tamanho fixo (uint32_t)
#include <cstddef>              // size_t

// Definição da estrutura diretamente no cpp
constexpr const char* SHM_NAME = "Local\\SharedMemory";  // Nome do objeto de memória mapeada ("Local\" = visível só na sessão atual)
constexpr const char* MUTEX_NAME = "Local\\SharedMutex"; // Nome do mutex (também na sessão atual)
constexpr size_t MAX_TEXT_SIZE = 1001;                   // Capacidade do buffer de texto (entrada de usuário)
std::atomic<bool> running(true);                         // Variável que verifica se alguém digitou "0". Atômica pois lida com a condição de corrida

struct SharedBuffer {
    uint32_t magic;            // “assinatura” para saber se a memória já foi inicializado (0xC0FFEE01 quando ok)
    char text[MAX_TEXT_SIZE];  // área de texto compartilhada entre processos
    size_t text_length;        // tamanho útil atual do texto (quantos bytes válidos em 'text')
    bool exit_flag;             // flag para encerramento global
};

#ifndef MY_ID
#define MY_ID 1 // chatA
// #define MY_ID 2 // chatB para o outro cpp
#endif
// A macro MY_ID identifica a instância (1 ou 2). 

// Lê do buffer compartilhado
std::string read_text(SharedBuffer* shm, HANDLE hMutex) {
    std::string result;                        // Cria uma variávrel para armazenar o valor da saída 
    WaitForSingleObject(hMutex, INFINITE);     // Agurda a liberação do Mutex

    if (shm->text_length > 0) {                // Verifica se há alguma coisa no Buffer
        result.assign(shm->text, shm->text_length); // assign() é um método da classe string que substitui o conteúdo de result pelo char text definido pelo tamanho text_length
    }

    ReleaseMutex(hMutex);                       // Libera o Mutex
    return result;                              // Retorna o texto lido (ou vazio)
}

// Escreve no buffer compartilhado
void write_text(SharedBuffer* shm, HANDLE hMutex, const std::string& text) {
    WaitForSingleObject(hMutex, INFINITE);     // Realiza a posse do mutex após liberado

    size_t len = text.size();                  // Calcula tamanho de entrada
    memcpy(shm->text, text.c_str(), len);      // Copia 'len' bytes da string para o buffer compartilhado. c.str() é um ponteiro para os caracteres da string
    shm->text[len] = '\0';                     // Adiciona um terminador de string ao final
    shm->text_length = len;                    // Atualiza o tamanho lógico

    std::string last;
    if (!text.empty() && text != last) { // Verifica se o último texto digitado não está vazio e se é diferente do anterior
        std::cout << "{\"processo\":" << MY_ID << ",\"mensagem\":\"[Processo A alterou o Buffer] " << text << "\"}" << std::endl;
        last = text;
    }

    ReleaseMutex(hMutex); // Libera o mutex
}

// Thread que faz a leitura contínua da struct do Buffer
void reader_thread(SharedBuffer* shm, HANDLE hMutex) {
    std::string last;
    while (running) {
        std::string text = read_text(shm, hMutex);
        if (!text.empty() && text != last) { // Verifica se o último texto digitado não está vazio e se é diferente do anterior
            std::cout << "{\"processo\":" << MY_ID << ",\"mensagem\":\"[Buffer atualizado] " << text << "\"}" << std::endl;
            last = text;
        }
        // Verifica flag de saída do Buffer
        WaitForSingleObject(hMutex, INFINITE);
        if (shm->exit_flag) { // Caso o flag de saída já esteja marcado encerra a Thread
            ReleaseMutex(hMutex);
            std::cout << "{\"processo\":" << MY_ID << ",\"mensagem\":\"[Alerta] Chat encerrado pelo outro processo!  Finalize o processo para encerrar." << "\"}" << std::endl;
            running = false;
            break;
        }
        ReleaseMutex(hMutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// Thread que monitora teclado
void input_thread(SharedBuffer* shm, HANDLE hMutex) {
    while (running) {
        std::string input;
        std::getline(std::cin, input);

        if (!running) break;

        if (input == "0") { // Caso o usuário tenha digitado 0, é marcado no flag da struct do Buffer true, sinalizando a saída
            WaitForSingleObject(hMutex, INFINITE);
            shm->exit_flag = true;
            ReleaseMutex(hMutex);
            std::cout << "{\"processo\":" << MY_ID << ",\"mensagem\":\"[Alerta] Chat encerrado!  Finalize o processo para encerrar." << "\"}" << std::endl;
            running = false; // Encerra a Thread
            break;
        }

        write_text(shm, hMutex, input);
    }
}

int main() {
    // Criar ou abrir memória compartilhada
    HANDLE hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,     // Parâmetro para especificar o Handle de memória física (arquivo). INVALID_HANDLE_VALUE → mapeamento em arquivo de paginação (memória anônima, área de memória na RAM)
        nullptr,                  // Segurança padrão
        PAGE_READWRITE,           // Proteção: leitura e escrita apenas (caso de Shared Memory)
        0,                        // Tamanho de objeto de memória inicial
        sizeof(SharedBuffer),     // Tamanho de objeto de memória final (assume o tamanho da estrutura do Buffer)
        SHM_NAME                  // Nome do objeto (permite que outro processo “abra” o mesmo segmento pelo nome), criado inicialmente nos dois processos
    );
    if (!hMap) { // Se CreateFileMappingA falhar, retorna null
        std::cerr << "Erro ao criar/abrir memória: " << GetLastError() << "\n"; // Em caso de falha, imprime código de erro do Windows
        return 1;                                                               // Sai com erro
    }

    SharedBuffer* shm = static_cast<SharedBuffer*>( // Ponteiro shm para o início da região de memória, tendo o formato da struct SharedBuffer
        MapViewOfFile(            // Função da API do Windows que mapeia e retorna um ponteiro do objeto da memória criada, sendo possível acessar direto a memória (ler/escrever)
            hMap,                 // Handle do mapeamento criado anteriormente
            FILE_MAP_ALL_ACCESS,  // Tipo de acesso leitura/escrita desejado à visão, necesse caso acesso completo
            0, 0,                 // Indica o deslocamento inicial dentro do objeto. Offset alto e baixo = 0 (começa no início)
            sizeof(SharedBuffer)  // Tamanho da visão (aqui igual ao tamanho da estrutura do Buffer)
        )
        );
    if (!shm) { // Se MapViewOfFile falhar, retorna null
        std::cerr << "Erro ao mapear memória: " << GetLastError() << "\n"; // MapViewOfFile falhou
        CloseHandle(hMap);                                                 // Fecha handle do mapeamento
        return 1;                                                          // Sai com erro
    }

    // Criar ou abrir mutex
    HANDLE hMutex = CreateMutexA( // Retorna um Handle do Mutex criado
        nullptr,   // Segurança padrão
        FALSE,     // Não adquirir posse inicial, ou seja, o processo não da lock inicialmente
        MUTEX_NAME // Nome do mutex (compartilhado por processos), criado inicialmente nos dois processos
    );
    if (!hMutex) { // Se CreateMutexA falhar, retorna null
        std::cerr << "Erro ao criar mutex: " << GetLastError() << "\n";
        UnmapViewOfFile(shm);  // Desmapeia visão
        CloseHandle(hMap);     // Fecha mapeamento
        return 1;              // Sai com erro
    }

    // Função da API do Windows usada para esperar até que o objeto de sincronização (Mutex) esteja liberado
    WaitForSingleObject(
        hMutex,     // Handle do Mutex
        INFINITE    // Tempo máximo para esperar, nesse caso infinito
    );

    if (shm->magic != 0xB16B00B5) {        // Se assinatura não está setada, esta é a primeira vez
        shm->magic = 0xB16B00B5;           // Marca como inicializado
        shm->text_length = 0;              // Zera tamanho lógico
        shm->text[0] = '\0';               // Coloca string vazia
        shm->exit_flag = false;
    }
    std::cout << "{\"processo\":" << MY_ID
        << ",\"mensagem\":\"Memoria inicializada: "
        << (shm->magic == 0xB16B00B5 ? "Sim" : "Nao")
        << "\"}" << std::endl;
    ReleaseMutex(hMutex);                  // Libera mutex

    std::cout << "{\"processo\":" << MY_ID
        << ",\"mensagem\":\"[Processo "
        << (MY_ID == 1 ? "A" : "B")
        << "] iniciado! Digite mensagens (0 para sair)\"}"
        << std::endl;
    // Mensagem de boas-vindas com identificação da instância (A ou B)
    std::thread reader(reader_thread, shm, hMutex);
    std::thread input_monitor(input_thread, shm, hMutex);

    // Aguarda as threads finalizarem
    reader.join();
    input_monitor.join();

    // Limpeza
    UnmapViewOfFile(shm);                    // Desmapeia visão (inacessível na prática porque não sai do loop)
    CloseHandle(hMap);                       // Fecha handle do mapeamento
    CloseHandle(hMutex);                     // Fecha handle do mutex

    return 0;                                // Fim do programa
}