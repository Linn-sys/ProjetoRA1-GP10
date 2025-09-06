#pragma once // Impede que o header seja incluído múltiplas vezes

#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

// Nomes globais para os objetos compartilhados
const TCHAR* NOME_MEMORIA = TEXT("MinhaMemoriaP2P");
const TCHAR* NOME_MUTEX = TEXT("MeuMutexP2P");

// Estrutura de dados compartilhada.
// Agora precisa de mais lógica para saber de quem é a mensagem.
struct DadosCompartilhados {
    bool deveSair;          // Flag para ambos os processos terminarem
    DWORD pidUltimoEscritor; // Armazena o ID do processo que escreveu por último
    char mensagem[256];
};