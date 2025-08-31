#include <windows.h> // 1. Inclui o cabeçalho principal da API do Windows, que contém todas as funções e estruturas para manipulação de processos, handles e pipes.
#include <iostream>  // 2. Inclui a biblioteca para operações de entrada e saída, como 'std::cout'.
#include <string>    // 3. Inclui a biblioteca para manipulação de strings.

int main() {
    HANDLE hReadPipe, hWritePipe; // 4. Declara duas variáveis do tipo 'HANDLE', que é um tipo de dado para representar um 'handle' ou 'identificador' de um objeto do sistema, como um pipe. 'hReadPipe' vai ser o handle da ponta de leitura do pipe, e 'hWritePipe' o de escrita.

    SECURITY_ATTRIBUTES sa; // 5. Declara uma variável do tipo 'SECURITY_ATTRIBUTES', uma estrutura de C que define se um handle pode ser herdado por um processo filho.
    sa.nLength = sizeof(sa); // 6. Atribui o tamanho da estrutura ao parâmetro 'nLength'. É uma prática padrão na API do Windows para garantir compatibilidade.
    sa.bInheritHandle = TRUE; // 7. 'bInheritHandle' é um booleano. Se 'TRUE', o handle que for criado com esta estrutura poderá ser herdado. Isso é essencial para o processo filho.
    sa.lpSecurityDescriptor = NULL; // 8. 'lpSecurityDescriptor' é um ponteiro para um descritor de segurança. 'NULL' indica que o handle terá o descritor de segurança padrão, herdado do processo pai.

    // 9. A função 'CreatePipe' cria o pipe.
    if (!CreatePipe(
        &hReadPipe,  // 9.1. Parâmetro de saída. A função vai preencher este ponteiro com o handle da ponta de leitura do pipe.
        &hWritePipe, // 9.2. Parâmetro de saída. A função vai preencher este ponteiro com o handle da ponta de escrita do pipe.
        &sa,         // 9.3. O ponteiro para a estrutura 'SECURITY_ATTRIBUTES' que criamos. Ele permite que as pontas do pipe sejam herdáveis.
        0            // 9.4. O tamanho do buffer do pipe. '0' indica que o sistema operacional deve usar o tamanho padrão.
    )) {
        std::cerr << "Erro ao criar o pipe.\n"; // 10. Se a função retornar 'FALSE', ocorreu um erro.
        return 1;
    }

    // 11. O pai não precisa que o processo filho herde a sua ponta de leitura, apenas a de escrita.
    SetHandleInformation(
        hReadPipe,           // 11.1. O handle que estamos modificando.
        HANDLE_FLAG_INHERIT, // 11.2. A flag que estamos alterando.
        0                    // 11.3. O novo valor da flag. '0' remove a capacidade de herdar.
    );

    STARTUPINFO si; // 12. 'STARTUPINFO' é uma estrutura que especifica as propriedades de um processo a ser criado, como seu estado de janela e seus handles padrão.
    PROCESS_INFORMATION pi; // 13. 'PROCESS_INFORMATION' é uma estrutura que recebe informações sobre o novo processo criado, como seu ID e handles de processo e thread.
    ZeroMemory(&si, sizeof(si)); // 14. Limpa a estrutura 'si' para garantir que não haja lixo de memória.
    si.cb = sizeof(si); // 15. Atribui o tamanho da estrutura a 'cb', um campo obrigatório.
    si.hStdOutput = hWritePipe; // 16. O 'hStdOutput' (handle de saída padrão) do processo filho será o handle de escrita do pipe.
    si.hStdError = hWritePipe;  // 17. O 'hStdError' (handle de erro padrão) do processo filho também será o handle de escrita do pipe.
    si.dwFlags |= STARTF_USESTDHANDLES; // 18. Esta flag ativa o uso dos handles padrão (entrada, saída e erro) que definimos na estrutura 'si'.

    ZeroMemory(&pi, sizeof(pi)); // 19. Limpa a estrutura 'pi' para o sistema a preencher.

    std::wstring childExePath = L"C:\\Users\\Usuario\\Documents\\ProjetoRA1-GP10\\Backend\\x64\\Debug\\ProcessoPipeB.exe"; // 20. O caminho para o executável do processo filho. 'L' indica uma string de caracteres largos.

    // 21. A função 'CreateProcess' cria o novo processo.
    if (!CreateProcess(
        NULL,                  // 21.1. Nome do módulo. 'NULL' usa o nome da linha de comando.
        &childExePath[0],      // 21.2. Linha de comando do processo. O L' na linha 20 converte a string para o formato necessário.
        NULL,                  // 21.3. Atributos de segurança do processo. 'NULL' usa o padrão.
        NULL,                  // 21.4. Atributos de segurança da thread principal. 'NULL' usa o padrão.
        TRUE,                  // 21.5. Booleano que indica se o processo filho deve herdar handles. 'TRUE' é essencial para que ele herde nosso pipe.
        0,                     // 21.6. Opções de criação. '0' usa o padrão.
        NULL,                  // 21.7. Ambiente. 'NULL' usa o ambiente do processo pai.
        NULL,                  // 21.8. Diretório de trabalho. 'NULL' usa o do processo pai.
        &si,                   // 21.9. Ponteiro para a estrutura 'STARTUPINFO' que define os handles padrão.
        &pi)                   // 21.10. Ponteiro para a estrutura 'PROCESS_INFORMATION' que será preenchida.
        ) {
        std::cerr << "Erro ao iniciar o processo filho. Código: " << GetLastError() << "\n";
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    //CloseHandle(hWritePipe); // 22. O processo pai não precisa mais da ponta de escrita do pipe, pois ela já foi herdada pelo filho. Fechá-la evita vazamento de recursos.

    std::string message = "Olá do processo pai!";
    DWORD bytesWritten;

    // 23. A função 'WriteFile' tenta escrever dados no pipe.
    if (!WriteFile(
        hWritePipe, // 23.1. O handle para o qual vamos escrever.
        message.c_str(), // 23.2. Um ponteiro para o buffer de dados a ser escrito.
        message.length(), // 23.3. O número de bytes a serem escritos.
        &bytesWritten, // 23.4. Ponteiro para uma variável que recebe o número de bytes realmente escritos.
        NULL) // 23.5. Ponteiro para a estrutura 'OVERLAPPED' para operações assíncronas. 'NULL' para operações síncronas.
        ) {
        std::cerr << "Erro ao escrever no pipe.\n";
    }
    CloseHandle(hWritePipe);

    char buffer[256];
    DWORD bytesRead;
    // 24. A função 'ReadFile' tenta ler dados do pipe.
    if (ReadFile(
        hReadPipe, // 24.1. O handle para o qual vamos ler.
        buffer, // 24.2. Um ponteiro para o buffer de dados que será lido.
        sizeof(buffer) - 1, // 24.3. O número máximo de bytes a serem lidos.
        &bytesRead, // 24.4. Ponteiro para uma variável que recebe o número de bytes realmente lidos.
        NULL) // 24.5. Ponteiro para 'OVERLAPPED'. 'NULL' para operações síncronas.
        ) {
        buffer[bytesRead] = '\0'; // 25. Adiciona um terminador nulo para que a string possa ser impressa.
        std::cout << "Pai: Recebi do filho -> " << buffer << "\n";
    }

    // 26. Fecha os handles restantes. Isso é crucial para liberar recursos do sistema.
    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}