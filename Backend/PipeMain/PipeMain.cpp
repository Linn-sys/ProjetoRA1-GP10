#include <windows.h>
#include <iostream>
#include <string>
#include <locale>

int main()
{
    //Cria os handles a serem utilizados no pipe
    HANDLE HandleLeitura, HandleEscrita;
    SECURITY_ATTRIBUTES AS;
    //Define os atributos de seguran�a para o processo a ser iniciado
    AS.nLength = sizeof(SECURITY_ATTRIBUTES);//Tamanho da estrutura, par�metro obrigat�rio pelo windows
    AS.lpSecurityDescriptor = NULL;//Descritor de seguran�a, nulo para saeguran�a padr�o
    AS.bInheritHandle = TRUE;//Permite que o processo derivado ou filho herde HANDLEs do pai

    if (!CreatePipe(&HandleLeitura,&HandleEscrita,&AS,5000)){
        //Cria o pipe, passando a refer�ncia dos 2 handles, a refer�ncia da estrutura de seguran�a e o tamanho sugerido do buffer  que � 5000 bytes
        //Sa�da formatada em JSON para o servidor NODE.JS
        std::cerr << "{\"processo\":1, \"mensagem\":\"Erro ao criar o pipe\"}" << std::endl;
        return 1;
    }

    SetHandleInformation(HandleEscrita, HANDLE_FLAG_INHERIT, 0);//Define o handle de escrita do pai como n�o herd�vel
    //Define as estruturas de informa��o de in�cio e geral do processo
    STARTUPINFO II;
    PROCESS_INFORMATION IP;

    ZeroMemory(&II, sizeof(II));//Limpa a estrutura para garantir que n�o h� lixo de mem�ria
    II.cb = sizeof(II);//Novamente, obrigat�rio passar o tamanho da estrutura
    II.dwFlags = STARTF_USESTDHANDLES;//Permite a configura��o manual � seguir dos handles do processo filho
    II.hStdInput = HandleLeitura;//Importante, define o handle de entrada como o handle de leitura do pipe
    II.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);//Define normalmente o output
    II.hStdError = GetStdHandle(STD_ERROR_HANDLE);//Define normalmente a sa�da de erros

    std::wstring C = L"PipeOffshoot.exe";//Cria uma string larga com o nome do execut�vel do filho
    WCHAR* PonteiroString = &C[0];//Cria um ponteiro para a primeira posi��o da string

    ZeroMemory(&IP, sizeof(IP));//Limpa a estrutura de informa��es do processo

    if (!CreateProcess(NULL,PonteiroString,NULL,NULL,TRUE,0,NULL,NULL,&II,&IP)) {
        //Cria o processo filho,
        //NULL para o primeiro par�metro, fun��o vai procurar o nome do m�dulo na linha de comando,
        //Ponteiro para para o nome do exe, permite que o windows procure ele no diret�rio de trabalho do pai,
        //NULL e NULL, n�o herda handle de processo nem de thread
        //TRUE, permite que os handles do pai sejam herdados,
        //0, nenhuma flag especial de cria��o necess�ria,
        //NULL e NULL, usa o PEB e diret�rio do processo pai,
        //&II e &IP, refer�ncia para ambas as estruturas que definem o in�cio e as caracter�sticas do processo, j� foram definidas anteriormente
        std::cerr << "{\"processo\":1, \"mensagem\":\"Erro ao criar o processo\"}" << std::endl;
        return 1;
    }

    CloseHandle(HandleLeitura);//Fewcha o handle de leitura do pai, desnecess�rio e feito para evitar deadlock

    DWORD BytesTransferidos;//Armazena quantos bytes s�o transferidos por mensagens

    std::wstring msg;
    while (true) {
        std::getline(std::wcin, msg);//Pega o input do usu�rio e guarda na string larga, garante compatibilidade unicode

        WriteFile(HandleEscrita, msg.c_str(),
            static_cast<DWORD>(msg.size() * sizeof(wchar_t)),
            &BytesTransferidos, NULL);//Envia por pipe os dados para o processo filho j� criado
        //Usa o handle de escrita para enviar dados,
        //msg.c_str(), retorna um ponteiro para a string ser acessada
        //tamanho em bytes do que vai ser enviado, como � em wchar a f�rmula � tamanho da lista * tamanho do wchar_t,
        //Refer�ncia para a vari�vel que armazena os bytes que foram transferidos,
        //NULL, determina opera��o s�ncrona no envio de dados

        if (msg == L"PARAR") {//Para a IPC via pipe manualmente
            std::wcout << L"{\"processo\":1, \"mensagem\":\"Finalizando PipeMain\"}" << std::endl;
            break;
        }

        //Confirma��o de envio
        std::wcout << L"{\"processo\":1, \"mensagem\":\"Enviou uma mensagem\"}" << std::endl;
    }

    //Espera indefinidamente at� que o processo filho encerre, e ent�o continua.
    WaitForSingleObject(IP.hProcess, INFINITE);

    //Fecha os handle de escrita do pai e os handles de processo e thread do filho, evitar gasto desnecess�rio de recursos
    CloseHandle(HandleEscrita);
    CloseHandle(IP.hProcess);
    CloseHandle(IP.hThread);
    return 0;
}