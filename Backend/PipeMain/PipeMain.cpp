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

    if (!CreatePipe(&HandleLeitura,&HandleEscrita,&AS,1025)){
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

    const WCHAR* C = L"C:\\Users\\SPIRIT INDIGO\\Documents\\Projetos\\CPP\\RA1\\ProjetoRA1-GP10\\Backend\\x64\\Debug\\PipeOffshoot.exe";

    ZeroMemory(&IP, sizeof(IP));

    if (!CreateProcess(C,NULL,NULL,NULL,TRUE,0,NULL,NULL,&II,&IP)) {
        std::cout << "Erro ao criar o processo..." << std::endl;
        return 1;
    }

    CloseHandle(HandleLeitura);

    DWORD BytesTransferidos;

    std::wcout << "COMUNICAÇÃO IPC ENTRE MAIN E OFFSHOOT" << "\n";
    while (true) {
        std::wstring msg;
        std::getline(std::wcin, msg);
        WriteFile(HandleEscrita, msg.c_str(), msg.size() * sizeof(wchar_t), &BytesTransferidos, NULL);
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