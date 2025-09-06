#pragma once
#include <cstdint>
#include <cstddef>

constexpr const char* SHM_NAME = "Local\\SharedMemoryExample";
constexpr const char* MUTEX_NAME = "Local\\SharedMutexExample";

constexpr size_t MAX_TEXT_SIZE = 1024;

struct SharedBuffer {
    uint32_t magic;            // indica se memória inicializada
    char text[MAX_TEXT_SIZE];  // texto compartilhado
    size_t text_length;        // tamanho atual do texto
};
