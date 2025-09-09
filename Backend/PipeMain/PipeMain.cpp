#include <windows.h>
#include <iostream>
#include <string>
#include <locale>

int main()
{
    //Cria os handles a serem utilizados no pipe
    HANDLE HandleLeitura, HandleEscrita;
    SECURITY_ATTRIBUTES AS;
    //Define os atributos de segurança para o processo a ser iniciado
    AS.nLength = sizeof(SECURITY_ATTRIBUTES);//Tamanho da estrutura, parâmetro obrigatório pelo windows
    AS.lpSecurityDescriptor = NULL;//Descritor de segurança, nulo para saegurança padrão
    AS.bInheritHandle = TRUE;//Permite que o processo derivado ou filho herde HANDLEs do pai

    if (!CreatePipe(&HandleLeitura,&HandleEscrita,&AS,5000)){
        //Cria o pipe, passando a referência dos 2 handles, a referência da estrutura de segurança e o tamanho sugerido do buffer  que é 5000 bytes
        //Saída formatada em JSON para o servidor NODE.JS
        std::cerr << "{\"processo\":1, \"mensagem\":\"Erro ao criar o pipe\"}" << std::endl;
        return 1;
    }

    SetHandleInformation(HandleEscrita, HANDLE_FLAG_INHERIT, 0);//Define o handle de escrita do pai como não herdável
    //Define as estruturas de informação de início e geral do processo
    STARTUPINFO II;
    PROCESS_INFORMATION IP;

    ZeroMemory(&II, sizeof(II));//Limpa a estrutura para garantir que não há lixo de memória
    II.cb = sizeof(II);//Novamente, obrigatório passar o tamanho da estrutura
    II.dwFlags = STARTF_USESTDHANDLES;//Permite a configuração manual à seguir dos handles do processo filho
    II.hStdInput = HandleLeitura;//Importante, define o handle de entrada como o handle de leitura do pipe
    II.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);//Define normalmente o output
    II.hStdError = GetStdHandle(STD_ERROR_HANDLE);//Define normalmente a saída de erros

    std::wstring C = L"PipeOffshoot.exe";//Cria uma string larga com o nome do executável do filho
    WCHAR* PonteiroString = &C[0];//Cria um ponteiro para a primeira posição da string

    ZeroMemory(&IP, sizeof(IP));//Limpa a estrutura de informações do processo

    if (!CreateProcess(NULL,PonteiroString,NULL,NULL,TRUE,0,NULL,NULL,&II,&IP)) {
        //Cria o processo filho,
        //NULL para o primeiro parâmetro, função vai procurar o nome do módulo na linha de comando,
        //Ponteiro para para o nome do exe, permite que o windows procure ele no diretório de trabalho do pai,
        //NULL e NULL, não herda handle de processo nem de thread
        //TRUE, permite que os handles do pai sejam herdados,
        //0, nenhuma flag especial de criação necessária,
        //NULL e NULL, usa o PEB e diretório do processo pai,
        //&II e &IP, referência para ambas as estruturas que definem o início e as características do processo, já foram definidas anteriormente
        std::cerr << "{\"processo\":1, \"mensagem\":\"Erro ao criar o processo\"}" << std::endl;
        return 1;
    }

    CloseHandle(HandleLeitura);//Fewcha o handle de leitura do pai, desnecessário e feito para evitar deadlock

    DWORD BytesTransferidos;//Armazena quantos bytes são transferidos por mensagens

    std::wstring msg;
    while (true) {
        std::getline(std::wcin, msg);//Pega o input do usuário e guarda na string larga, garante compatibilidade unicode

        WriteFile(HandleEscrita, msg.c_str(),
            static_cast<DWORD>(msg.size() * sizeof(wchar_t)),
            &BytesTransferidos, NULL);//Envia por pipe os dados para o processo filho já criado
        //Usa o handle de escrita para enviar dados,
        //msg.c_str(), retorna um ponteiro para a string ser acessada
        //tamanho em bytes do que vai ser enviado, como é em wchar a fórmula é tamanho da lista * tamanho do wchar_t,
        //Referência para a variável que armazena os bytes que foram transferidos,
        //NULL, determina operação síncrona no envio de dados

        if (msg == L"PARAR") {//Para a IPC via pipe manualmente
            std::wcout << L"{\"processo\":1, \"mensagem\":\"Finalizando PipeMain\"}" << std::endl;
            break;
        }

        //Confirmação de envio
        std::wcout << L"{\"processo\":1, \"mensagem\":\"Enviou uma mensagem\"}" << std::endl;
    }

    //Espera indefinidamente até que o processo filho encerre, e então continua.
    WaitForSingleObject(IP.hProcess, INFINITE);

    //Fecha os handle de escrita do pai e os handles de processo e thread do filho, evitar gasto desnecessário de recursos
    CloseHandle(HandleEscrita);
    CloseHandle(IP.hProcess);
    CloseHandle(IP.hThread);
    return 0;
}