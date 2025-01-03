#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include "cache_lib.h"

//int add(int a, int b) {
//    return a + b;
//}
//
//void greet(const char* name) {
//    std::cout << "Hello, " << name << " , I believe in you!" << std::endl;
//}

#define BLOCK_SIZE 512

typedef struct CacheEntry {
    int block_id;
    char* data;
    bool ref_bit;
} CacheEntry;

typedef struct Cache {
    size_t capacity;
    CacheEntry* entries;
    int clock_hand;
    size_t size;
    HANDLE mutex;
} Cache;

Cache* cache = nullptr;
HANDLE disk_handle;

bool cache_read(int block_id, void* buffer) {
    // 1. Проверка на валидность кэша:
    if (cache == nullptr) return false;

    // 2. Захват мьютекса:
    WaitForSingleObject(cache->mutex, INFINITE);

    // 3. Поиск блока в кэше:
    for (size_t i = 0; i < cache->size; ++i) {
        if (cache->entries[i].block_id == block_id) {
            // 4a. Кэш-хит:
            cache->entries[i].ref_bit = true; // Устанавливаем бит использования
            memcpy(buffer, cache->entries[i].data, BLOCK_SIZE); // Копируем данные в буфер
            ReleaseMutex(cache->mutex); // Освобождаем мьютекс
            return true; // Возвращаем true (данные получены из кэша)
        }
    }

    // 5. Кэш-промах: (блок не найден в кэше)
    DWORD bytesRead;
    LARGE_INTEGER offset;
    offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;

    // 6. Позиционируем файловый указатель:
    SetFilePointerEx(disk_handle, //дескриптор файла, в нашем случае, "стрелка"
                    offset, // смещение указателя
                    NULL, //новый указатель позиции
                    FILE_BEGIN); //точка отсчёта

    // 7. Читаем данные с диска:
    if(!ReadFile(disk_handle, buffer, BLOCK_SIZE, &bytesRead, NULL))
    {
        ReleaseMutex(cache->mutex); // Освобождаем мьютекс (в случае ошибки чтения)
        return false; // Возвращаем false (ошибка чтения)
    }

    // 8. Размещение в кэше новой страницы:
    if (cache->size < cache->capacity)
    {
        // 8a. Есть свободные места:
        cache->entries[cache->size].block_id = block_id; // Записываем block_id
        cache->entries[cache->size].data = (char*)malloc(BLOCK_SIZE); // Выделяем память
       if (cache->entries[cache->size].data == nullptr){
            ReleaseMutex(cache->mutex); //Освобождаем мьютекс
            return false; // Не удалось выделить память
        }
        memcpy(cache->entries[cache->size].data, buffer, BLOCK_SIZE); // Копируем данные в кэш
        cache->entries[cache->size].ref_bit = true;  // Устанавливаем бит использования
        cache->size++;  // Увеличиваем размер кэша
    } else {
         // 8b. Нет свободного места, используем Clock:
        while (true) {
             if (cache->entries[cache->clock_hand].ref_bit == true) {
                cache->entries[cache->clock_hand].ref_bit = false;
                cache->clock_hand = (cache->clock_hand + 1) % cache->capacity;
            } else {
                free(cache->entries[cache->clock_hand].data);
                 cache->entries[cache->clock_hand].block_id = block_id;
                cache->entries[cache->clock_hand].data = (char*)malloc(BLOCK_SIZE);
                 if (cache->entries[cache->clock_hand].data == nullptr){
                       ReleaseMutex(cache->mutex);
                       return false; //не удалось выделить память
                 }
                  memcpy(cache->entries[cache->clock_hand].data, buffer, BLOCK_SIZE);
                cache->entries[cache->clock_hand].ref_bit = true;
                 cache->clock_hand = (cache->clock_hand + 1) % cache->capacity;
                break;
            }
        }
    }
    ReleaseMutex(cache->mutex); // 9. Освобождаем мьютекс
    return true; // Возвращаем true (чтение выполнено успешно)
}

bool cache_write(int block_id, const void* buffer) {
    // 1. Проверка на валидность кэша:
    if (cache == nullptr) return false;

    // 2. Захват мьютекса:
    WaitForSingleObject(cache->mutex, INFINITE);

    // 3. Поиск блока в кэше:
    for (size_t i = 0; i < cache->size; ++i) {
        if (cache->entries[i].block_id == block_id) {
            // 4a. Кэш-хит:
            memcpy(cache->entries[i].data, buffer, BLOCK_SIZE); // Обновляем данные в кэше
             // 5. Запись на диск:
            LARGE_INTEGER offset;
            offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;
           SetFilePointerEx(disk_handle, offset, NULL, FILE_BEGIN); // Позиционируем файловый указатель
             DWORD bytesWritten;
              if (!WriteFile(disk_handle, buffer, BLOCK_SIZE, &bytesWritten, NULL)) // Записываем данные на диск
             {
                  ReleaseMutex(cache->mutex); // Освобождаем мьютекс (в случае ошибки записи)
                 return false; // Возвращаем false (ошибка записи)
            }
             ReleaseMutex(cache->mutex); // 6. Освобождаем мьютекс
            return true;  // Возвращаем true (запись успешна)
        }
    }

    // 7. Кэш-промах: (блок не найден в кэше)
     // 8. Записываем данные на диск:
    DWORD bytesWritten;
    LARGE_INTEGER offset;
    offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;
     SetFilePointerEx(disk_handle, offset, NULL, FILE_BEGIN); // Позиционируем файловый указатель
     if (!WriteFile(disk_handle, buffer, BLOCK_SIZE, &bytesWritten, NULL)) // Записываем данные на диск
    {
        ReleaseMutex(cache->mutex); // Освобождаем мьютекс (в случае ошибки записи)
         return false; // Возвращаем false (ошибка записи)
    }

    // 9. Размещение в кэше новой страницы:
    if (cache->size < cache->capacity)
    {
        // 9a. Есть свободные места:
        cache->entries[cache->size].block_id = block_id; // Записываем block_id
        cache->entries[cache->size].data = (char*)malloc(BLOCK_SIZE); // Выделяем память
         if (cache->entries[cache->size].data == nullptr){
               ReleaseMutex(cache->mutex); // Освобождаем мьютекс (не удалось выделить память)
               return false; // Возвращаем false (не удалось выделить память)
        }
        memcpy(cache->entries[cache->size].data, buffer, BLOCK_SIZE); // Копируем данные в кэш
         cache->entries[cache->size].ref_bit = true; // Устанавливаем бит использования
         cache->size++; // Увеличиваем размер кэша
     } else {
          // 9b. Нет свободного места, используем Clock:
          while (true) {
            if (cache->entries[cache->clock_hand].ref_bit == true) {
                cache->entries[cache->clock_hand].ref_bit = false;
                 cache->clock_hand = (cache->clock_hand + 1) % cache->capacity;
            } else {
               free(cache->entries[cache->clock_hand].data); // Освобождаем старые данные
               cache->entries[cache->clock_hand].block_id = block_id; // Записываем block_id
               cache->entries[cache->clock_hand].data = (char*)malloc(BLOCK_SIZE); // Выделяем память
                if (cache->entries[cache->clock_hand].data == nullptr){
                     ReleaseMutex(cache->mutex);
                    return false; // Возвращаем false (не удалось выделить память)
                }
                 memcpy(cache->entries[cache->clock_hand].data, buffer, BLOCK_SIZE); // Копируем новые данные
                cache->entries[cache->clock_hand].ref_bit = true;  // Устанавливаем бит использования
                cache->clock_hand = (cache->clock_hand + 1) % cache->capacity; // Обновляем clock_hand
                 break;
            }
        }
     }
       ReleaseMutex(cache->mutex); // 10. Освобождаем мьютекс
    return true; // 11. Возвращаем true (запись выполнена)
}

bool cache_init(size_t capacity, const char* disk_file) {
    if (cache != nullptr) {
        return false; // Кэш уже проинициализирован
    }

    cache = (Cache*)malloc(sizeof(Cache));
    if (cache == nullptr) {
         return false;
    }
    cache->capacity = capacity;
    cache->entries = (CacheEntry*)malloc(sizeof(CacheEntry) * capacity);
    if (cache->entries == nullptr) {
        free(cache);
        return false;
    }
    for (size_t i = 0; i < capacity; ++i) {
        cache->entries[i].data = nullptr;
    }
    cache->clock_hand = 0;
    cache->size = 0;
    cache->mutex = CreateMutex(NULL, FALSE, NULL);
    if (cache->mutex == NULL) {
        free(cache->entries);
        free(cache);
        return false;
    }
    disk_handle = CreateFileA(disk_file, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (disk_handle == INVALID_HANDLE_VALUE) {
        CloseHandle(cache->mutex);
        free(cache->entries);
        free(cache);
        return false;
    }

    return true;
}

void cache_destroy() {
    if (cache == nullptr) return;
    WaitForSingleObject(cache->mutex, INFINITE);
    for (size_t i = 0; i < cache->capacity; ++i) {
         if (cache->entries[i].data != nullptr)
             free(cache->entries[i].data);
    }
    free(cache->entries);
    CloseHandle(disk_handle);
    CloseHandle(cache->mutex);
     free(cache);
     cache = nullptr;
}