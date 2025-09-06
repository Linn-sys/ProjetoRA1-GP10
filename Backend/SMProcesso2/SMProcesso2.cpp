#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cstddef>

// Defini��o da estrutura diretamente no cpp
constexpr const char* SHM_NAME = "Local\\SharedMemoryExample";
constexpr const char* MUTEX_NAME = "Local\\SharedMutexExample";
constexpr size_t MAX_TEXT_SIZE = 1024;

struct SharedBuffer {
    uint32_t magic;            // indica se mem�ria inicializada
    char text[MAX_TEXT_SIZE];  // texto compartilhado
    size_t text_length;        // tamanho atual do texto
};

#ifndef MY_ID
#define MY_ID 2 // chatB
// #define MY_ID 2 // chatB para o outro cpp
#endif

// Escreve no buffer compartilhado
void write_text(SharedBuffer* shm, HANDLE hMutex, const std::string& text) {
    WaitForSingleObject(hMutex, INFINITE);

    size_t len = text.size();
    if (len > MAX_TEXT_SIZE) len = MAX_TEXT_SIZE;

    memcpy(shm->text, text.c_str(), len);
    shm->text[len] = '\0';
    shm->text_length = len;

    ReleaseMutex(hMutex);
}

// L� do buffer compartilhado
std::string read_text(SharedBuffer* shm, HANDLE hMutex) {
    std::string result;
    WaitForSingleObject(hMutex, INFINITE);

    if (shm->text_length > 0) {
        result.assign(shm->text, shm->text_length);
    }

    ReleaseMutex(hMutex);
    return result;
}

// Mostra estado da mem�ria e mutex
void print_status(SharedBuffer* shm, HANDLE hMutex) {
    DWORD wait = WaitForSingleObject(hMutex, 0); // n�o bloqueia
    bool locked = (wait == WAIT_TIMEOUT) ? true : false;

    std::cout << "--------------------------\n";
    std::cout << "Memoria inicializada: " << (shm->magic == 0xC0FFEE01 ? "Sim" : "Nao") << "\n";
    std::cout << "Texto atual: " << std::string(shm->text, shm->text_length) << "\n";
    std::cout << "Mutex travado: " << (locked ? "Sim" : "Nao") << "\n";
    std::cout << "--------------------------\n";

    if (!locked) ReleaseMutex(hMutex); // libera se conseguimos pegar
}

int main() {
    // Criar ou abrir mem�ria compartilhada
    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SharedBuffer), SHM_NAME);
    if (!hMap) {
        std::cerr << "Erro ao criar/abrir mem�ria: " << GetLastError() << "\n";
        return 1;
    }

    SharedBuffer* shm = static_cast<SharedBuffer*>(MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBuffer)));
    if (!shm) {
        std::cerr << "Erro ao mapear mem�ria: " << GetLastError() << "\n";
        CloseHandle(hMap);
        return 1;
    }

    // Criar ou abrir mutex
    HANDLE hMutex = CreateMutexA(nullptr, FALSE, MUTEX_NAME);
    if (!hMutex) {
        std::cerr << "Erro ao criar mutex: " << GetLastError() << "\n";
        UnmapViewOfFile(shm);
        CloseHandle(hMap);
        return 1;
    }

    // Inicializa��o �nica
    WaitForSingleObject(hMutex, INFINITE);
    if (shm->magic != 0xC0FFEE01) {
        shm->magic = 0xC0FFEE01;
        shm->text_length = 0;
        shm->text[0] = '\0';
    }
    ReleaseMutex(hMutex);

    std::cout << "[Chat " << (MY_ID == 1 ? "A" : "B") << "] iniciado! Digite mensagens (Ctrl+C para sair)\n";

    // Thread de leitura cont�nua
    std::thread reader([&]() {
        std::string last;
        while (true) {
            std::string text = read_text(shm, hMutex);
            if (!text.empty() && text != last) {
                last = text;
                std::cout << "\n[Recebido] " << text << "\n> ";
                print_status(shm, hMutex);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        });

    // Loop principal de escrita
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        write_text(shm, hMutex, input);
        print_status(shm, hMutex);
    }

    reader.join();

    // Limpeza
    UnmapViewOfFile(shm);
    CloseHandle(hMap);
    CloseHandle(hMutex);

    return 0;
}
