#include <windows.h>
#include <iostream>
#include <string>
#include <locale>

int main()
{
    HANDLE HandleLeitura, HandleEscrita;
    SECURITY_ATTRIBUTES AS;
    //SECURITY_DESCRIPTOR DS;

    AS.nLength = sizeof(SECURITY_ATTRIBUTES);
    AS.lpSecurityDescriptor = NULL;
    AS.bInheritHandle = TRUE;

    if (!CreatePipe(&HandleLeitura,&HandleEscrita,&AS,5000)){
        std::cout << "Erro ao criar o pipe..." << std::endl;
        return 1;
    }
    SetHandleInformation(HandleEscrita, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFO II;
    PROCESS_INFORMATION IP;

    ZeroMemory(&II, sizeof(II));
    II.cb = sizeof(II);
    II.dwFlags = STARTF_USESTDHANDLES;
    II.hStdInput = HandleLeitura;
    II.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    II.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    //const WCHAR* C = L"PipeOffshoot.exe";
    //std::wstring C = L"PipeOffshoot.exe";
    std::wstring C = L"PipeOffshoot.exe";
    WCHAR* PonteiroString = &C[0];

    ZeroMemory(&IP, sizeof(IP));

    if (!CreateProcess(NULL,PonteiroString,NULL,NULL,TRUE,0,NULL,NULL,&II,&IP)) {
        std::cout << "Erro ao criar o processo..." << std::endl;
        return 1;
    }

    CloseHandle(HandleLeitura);

    DWORD BytesTransferidos;

    WCHAR TESTE[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, TESTE);
    std::wcout << L"O diretório de PipeMain eh" << TESTE << "\n";
    std::wcout << "<<<Inter Process Communication iniciada>>" << "\n";
    while (true) {
        //std::cout << "Main enviou:" << std::endl;
        std::wstring msg;
        std::getline(std::wcin, msg);
        WriteFile(HandleEscrita, msg.c_str(), static_cast<DWORD>(msg.size() * sizeof(wchar_t)), &BytesTransferidos, NULL);
        if (msg == L"PARAR") {
            break;
        }
    }

    WaitForSingleObject(IP.hProcess, INFINITE);//Espera pelo filho

    CloseHandle(HandleEscrita);
    //Fecha os handles do filho (processo e thread)
    CloseHandle(IP.hProcess);
    CloseHandle(IP.hThread);
    return 0;
}