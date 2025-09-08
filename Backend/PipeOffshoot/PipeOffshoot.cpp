#include <windows.h>
#include <iostream>
#include <string>
#include <locale>

int main() {
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    if (entrada == INVALID_HANDLE_VALUE) {
        std::cerr << "{\"processo\":2, \"mensagem\":\"Handle inválido\"}" << std::endl;
        return 1;
    }

    std::wstring MensagemEnviada;
    wchar_t BufferTemp[5000];
    DWORD BytesLidos;

    while (true) {

        if (!ReadFile(entrada, BufferTemp, sizeof(BufferTemp), &BytesLidos, NULL) || BytesLidos == 0) {
            continue;
        }

        DWORD NumChar = BytesLidos / sizeof(wchar_t);
        MensagemEnviada.append(BufferTemp, NumChar);

        
        if (MensagemEnviada == L"PARAR") {
            std::wcout << L"{\"processo\":2, \"mensagem\":\"Destinatário encerrando\"}" << std::endl;
            break;
        }
        // Mostra o que recebeu
        std::wcout << L"{\"processo\":2, \"mensagem\":\"Recebi: " << MensagemEnviada << L"\"}" << std::endl;

        MensagemEnviada.clear();
    }

    return 0;
}
