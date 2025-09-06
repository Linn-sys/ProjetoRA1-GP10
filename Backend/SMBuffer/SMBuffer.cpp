#pragma once
#include <windows.h>
#include <string>

constexpr const char* SHM_NAME = "Local\\ChatSharedBuffer";
constexpr const char* MUTEX_NAME = "Local\\ChatSharedMutex";

constexpr size_t BUFFER_SIZE = 256;

struct SharedBuffer {
    char message[BUFFER_SIZE];
    int lastWriter;   // 0 = nenhum, 1 = A, 2 = B
    bool hasData;
};
