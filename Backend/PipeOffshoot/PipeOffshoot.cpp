#include <windows.h>   // API do Windows: GetStdHandle, ReadFile, HANDLE
#include <iostream>    // std::wcout, std::cerr
#include <string>      // std::wstring
#include <locale>      // suporte a localidade e Unicode

int main() {
    // Obtém o handle da entrada padrão (stdin)
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    if (entrada == INVALID_HANDLE_VALUE) { // Verifica se houve erro
        // Se inválido, retorna uma mensagem JSON de erro e sai
        std::cerr << "{\"processo\":2, \"mensagem\":\"Handle inválido\"}" << std::endl;
        return 1;
    }

    // Variável para armazenar a mensagem completa recebida
    std::wstring MensagemEnviada;
    // Buffer temporário de leitura (capacidade 5000 wchar_t)
    wchar_t BufferTemp[5000];
    // Armazena a quantidade de bytes lidos em cada chamada de ReadFile
    DWORD BytesLidos;

    // Loop infinito para ler continuamente da entrada padrão
    while (true) {

        // Lê do handle 'entrada' (stdin) para BufferTemp
        // ReadFile retorna FALSE em caso de erro ou BytesLidos = 0 se nada foi lido
        if (!ReadFile(entrada, BufferTemp, sizeof(BufferTemp), &BytesLidos, NULL) || BytesLidos == 0) {
            continue; // Se não leu nada, ignora e tenta novamente
        }

        // Converte número de bytes lidos em número de wchar_t
        DWORD NumChar = BytesLidos / sizeof(wchar_t);
        // Adiciona os caracteres lidos ao final de MensagemEnviada
        MensagemEnviada.append(BufferTemp, NumChar);

        // Se o usuário digitar "PARAR", encerra o loop
        if (MensagemEnviada == L"PARAR") {
            // Mostra mensagem JSON indicando encerramento do destinatário
            std::wcout << L"{\"processo\":2, \"mensagem\":\"Destinatário encerrando\"}" << std::endl;
            break;
        }

        // Mostra o que foi recebido em formato JSON
        std::wcout << L"{\"processo\":2, \"mensagem\":\"Recebi: "
            << MensagemEnviada << L"\"}" << std::endl;

        // Limpa a variável para próxima leitura
        MensagemEnviada.clear();
    }

    return 0; // Sai do programa
}
