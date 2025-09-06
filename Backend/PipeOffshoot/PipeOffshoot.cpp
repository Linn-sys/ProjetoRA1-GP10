#include <windows.h>
#include <iostream>
#include <string>
#include <locale>

int main() {
    HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    if (entrada == INVALID_HANDLE_VALUE) {
        std::cout << "O Handle de leitura é inválido..." << std::endl;
        return 1;
    }

    std::wstring MensagemEnviada;
    wchar_t BufferTemp[5000];
    DWORD BytesLidos;

    while (true) {

        ReadFile(entrada, BufferTemp, sizeof(BufferTemp), &BytesLidos, NULL);

        DWORD NumChar = BytesLidos / sizeof(wchar_t);

        MensagemEnviada.append(BufferTemp, NumChar);

        
        if (MensagemEnviada == L"PARAR") {
            std::wcout << L"Offshoot recebeu: " << MensagemEnviada << "\n";
            std::wcout << "<<<Inter Process Communication encerrada>>>\n";
            break;
        }
        std::wcout << L"Offshoot recebeu: " << MensagemEnviada << "/Tamanho da mensagem em bytes: "<< BytesLidos << "\n";
        MensagemEnviada.clear();
    }

    return 0;
}
