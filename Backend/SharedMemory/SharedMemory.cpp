#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include <windows.h>
#include <processthreadsapi.h>
#include <tchar.h> // Inclui o cabeçalho para as macros TCHAR

// A estrutura compartilhada permanece a mesma
struct SharedData {
    int contador;
    bool nova_mensagem;
};

// A função principal agora usa _tmain e TCHAR para os argumentos
int _tmain(int argc, TCHAR* argv[]) {
    // Usa a macro TEXT() para definir os nomes. Isso os torna LPCWSTR em modo Unicode.
    const TCHAR* SHARED_MEM_NAME = TEXT("MyNativeWindowsSharedMemory");
    const TCHAR* MUTEX_NAME = TEXT("MyNativeWindowsMutex");
    const int MAX_COUNT = 10;

    HANDLE hMapFile;
    HANDLE hMutex;
    SharedData* pSharedData = nullptr;

    if (argc == 1) {
        // --- MODO ESCRITOR (CRIADOR) ---
        std::cout << "[Escritor] Modo Criador. Preparando..." << std::endl;

        hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
        if (hMutex == NULL) { /* ... tratamento de erro ... */ return 1; }

        hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(SharedData),
            SHARED_MEM_NAME);

        if (hMapFile == NULL) { /* ... tratamento de erro ... */ CloseHandle(hMutex); return 1; }

        pSharedData = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
        if (pSharedData == NULL) { /* ... tratamento de erro ... */ CloseHandle(hMapFile); CloseHandle(hMutex); return 1; }

        pSharedData->contador = -1;
        pSharedData->nova_mensagem = false;

        // Para a linha de comando, usamos um buffer TCHAR
        TCHAR command_line[MAX_PATH];
        // Usamos uma função segura para formatar a string
        _stprintf_s(command_line, MAX_PATH, TEXT("\"%s\" reader"), argv[0]);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "CreateProcess falhou. Erro: " << GetLastError() << std::endl;
            // Limpeza
            UnmapViewOfFile(pSharedData);
            CloseHandle(hMapFile);
            CloseHandle(hMutex);
            return 1;
        }
        std::cout << "[Escritor] Processo Leitor iniciado." << std::endl;

        for (int i = 0; i < MAX_COUNT; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            WaitForSingleObject(hMutex, INFINITE);
            pSharedData->contador = i;
            pSharedData->nova_mensagem = true;
            std::cout << "[Escritor] Escreveu o valor: " << i << std::endl;
            ReleaseMutex(hMutex);
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        UnmapViewOfFile(pSharedData);
        CloseHandle(hMapFile);
        CloseHandle(hMutex);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "[Escritor] Processo Leitor terminou. Encerrando." << std::endl;

    }
    else if (argc == 2 && _tcscmp(argv[1], TEXT("reader")) == 0) { // Usa _tcscmp para comparar TCHAR
        // --- MODO LEITOR (CLIENTE) ---
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
        if (hMutex == NULL) { /* ... tratamento de erro ... */ return 1; }

        hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        if (hMapFile == NULL) { /* ... tratamento de erro ... */ CloseHandle(hMutex); return 1; }

        pSharedData = (SharedData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));
        if (pSharedData == NULL) { /* ... tratamento de erro ... */ CloseHandle(hMapFile); CloseHandle(hMutex); return 1; }

        std::cout << "[Leitor] Conectado. Aguardando dados..." << std::endl;

        int ultimo_valor_lido = -1;
        while (ultimo_valor_lido < MAX_COUNT - 1) {
            WaitForSingleObject(hMutex, INFINITE);
            if (pSharedData->nova_mensagem) {
                ultimo_valor_lido = pSharedData->contador;
                // Para imprimir wchar_t no cout, é melhor usar wcout
                std::wcout << L"[Leitor] Leu o valor: " << ultimo_valor_lido << std::endl;
                pSharedData->nova_mensagem = false;
            }
            ReleaseMutex(hMutex);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        UnmapViewOfFile(pSharedData);
        CloseHandle(hMapFile);
        CloseHandle(hMutex);
        std::wcout << L"[Leitor] Tarefa concluida. Encerrando." << std::endl;
    }

    return 0;
}