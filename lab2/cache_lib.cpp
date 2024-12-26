#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include "cache_lib.h"

using namespace std;

//int add(int a, int b) {
//    return a + b;
//}
//
//void greet(const char* name) {
//    std::cout << "Hello, " << name << " , I believe in you!" << std::endl;
//}

#define BLOCK_SIZE 512
#define MAX_OPEN_FILES 10

typedef struct CacheEntry {
    int block_id; // ID странциы
    char* data; // Данные
    bool ref_bit; //Бит использования (для осуществления Clock)
} CacheEntry;

typedef struct Cache {
    size_t capacity; //размер
    CacheEntry* entries; //вместительность кэша
    int clock_hand; // указатель ("стрелка часов")
    size_t size; //текущий размер кэша
    HANDLE mutex; //мьютекс
} Cache;

Cache* cache = nullptr;
HANDLE disk_handle;

bool cache_read(int block_id, void* buffer) {
    cout << "Trying to read block " << block_id << endl;
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
            cout << "Block is currently in cache. " << endl;
            ReleaseMutex(cache->mutex);
            return true;
        }
    }

    // 5. Кэш-промах: (блок не найден в кэше)
    DWORD bytesRead;
    LARGE_INTEGER offset;
    offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;

    // 6. Позиционируем файловый указатель:
    SetFilePointerEx(disk_handle, //дескриптор файла, в нашем случае, "стрелка"
                    offset, //смещение указателя
                    NULL, //новый указатель позиции
                    FILE_BEGIN); //точка отсчёта

    // 7. Читаем данные с диска:
    if(!ReadFile(disk_handle, buffer, BLOCK_SIZE, &bytesRead, NULL))
    {
        ReleaseMutex(cache->mutex);
        return false;
    }

    // 8. Размещение в кэше новой страницы:
    if (cache->size < cache->capacity)
    {
        // 8a. Есть свободные места:
        cout << "Current cache size: " << cache->size << endl;
        cache->entries[cache->size].block_id = block_id; // Записываем block_id
        cache->entries[cache->size].data = (char*)malloc(BLOCK_SIZE); // Выделяем память
       if (cache->entries[cache->size].data == nullptr){
            ReleaseMutex(cache->mutex); //Освобождаем мьютекс
            return false; // Не удалось выделить память
        }
        memcpy(cache->entries[cache->size].data, buffer, BLOCK_SIZE); // Копируем данные в кэш
        cache->entries[cache->size].ref_bit = true;  // Устанавливаем бит использования
        cache->size++;  // Увеличиваем размер кэша
        cout << "New block have been added in cache. " << endl;
        cache->clock_hand = (cache->clock_hand + 1) % (cache->capacity);
        cout << "Pointer current position: " << cache->clock_hand << endl;
    } else {
         // 8b. Нет свободного места, используем Clock:
        while (true) {
             if (cache->entries[cache->clock_hand].ref_bit == true) {
                cache->entries[cache->clock_hand].ref_bit = false;
                cache->clock_hand = (cache->clock_hand + 1) % (cache->capacity);
                cout << "Pointer current position: " << cache->clock_hand << endl;
            } else {
                free(cache->entries[cache->clock_hand].data);
                 cache->entries[cache->clock_hand].block_id = block_id;
                cache->entries[cache->clock_hand].data = (char*)malloc(BLOCK_SIZE);
                 if (cache->entries[cache->clock_hand].data == nullptr){
                       ReleaseMutex(cache->mutex);
                       return false;
                 }
                  memcpy(cache->entries[cache->clock_hand].data, buffer, BLOCK_SIZE);
                 cache->entries[cache->clock_hand].ref_bit = true;
                 cout << "Cache block has been rewritten " << cache->clock_hand << endl;
                 cache->clock_hand = (cache->clock_hand + 1) % cache->capacity;
                 cout << "Pointer current position: " << cache->clock_hand << endl;
                break;
            }
        }
    }
    ReleaseMutex(cache->mutex);
    return true;
}

bool cache_write(int block_id, const void* buffer) {
    cout << "Trying to write block " << block_id << endl;
    // 1. Проверка на валидность кэша:
    if (cache == nullptr) return false;

    // 2. Захват мьютекса:
    WaitForSingleObject(cache->mutex, INFINITE);

    // 3. Поиск блока в кэше:
    for (size_t i = 0; i < cache->size; ++i) {
        if (cache->entries[i].block_id == block_id) {
            cout << "Block is currently in cache. " << endl;
            // 4a. Кэш-хит:
            memcpy(cache->entries[i].data, buffer, BLOCK_SIZE);
             // 5. Запись на диск:
            LARGE_INTEGER offset;
            offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;
           SetFilePointerEx(disk_handle, offset, NULL, FILE_BEGIN);
             DWORD bytesWritten;
              if (!WriteFile(disk_handle, buffer, BLOCK_SIZE, &bytesWritten, NULL))
             {
                  ReleaseMutex(cache->mutex);
                 return false;
            }
             ReleaseMutex(cache->mutex);
            return true;
        }
    }

    // 6. Кэш-промах: (блок не найден в кэше)
     // 7. Записываем данные на диск:
    DWORD bytesWritten;
    LARGE_INTEGER offset;
    offset.QuadPart = (LONGLONG)block_id * BLOCK_SIZE;
     SetFilePointerEx(disk_handle, offset, NULL, FILE_BEGIN);
     if (!WriteFile(disk_handle, buffer, BLOCK_SIZE, &bytesWritten, NULL))
    {
        ReleaseMutex(cache->mutex);
         return false;
    }

    // 8. Размещение в кэше новой страницы:
    if (cache->size < cache->capacity)
    {
        // 8a. Есть свободные места:
        cout << "Current cache size: " << cache->size << endl;
        cache->entries[cache->size].block_id = block_id;
        cache->entries[cache->size].data = (char*)malloc(BLOCK_SIZE);
         if (cache->entries[cache->size].data == nullptr){
               ReleaseMutex(cache->mutex);
               return false;
        }
        memcpy(cache->entries[cache->size].data, buffer, BLOCK_SIZE);
         cache->entries[cache->size].ref_bit = true;
         cache->size++;
         cout << "New block have been added in cache. " << endl;
         cache->clock_hand = (cache->clock_hand + 1) % (cache->capacity);
         cout << "Pointer current position: " << cache->clock_hand << endl;
     } else {
          // 8b. Нет свободного места, используем Clock:
          while (true) {
            if (cache->entries[cache->clock_hand].ref_bit == true) {
                cache->entries[cache->clock_hand].ref_bit = false; //занулить бит использования
                 cache->clock_hand = (cache->clock_hand + 1) % (cache->capacity); //переместить указатель дальше
                 cout << "Pointer current position: " << cache->clock_hand << endl;
            } else {
               //нашли элемент без бита использования, надо бы заменить
               free(cache->entries[cache->clock_hand].data); // Освобождаем старые данные
               cache->entries[cache->clock_hand].block_id = block_id; // Новый id
               cache->entries[cache->clock_hand].data = (char*)malloc(BLOCK_SIZE);
                if (cache->entries[cache->clock_hand].data == nullptr){
                     ReleaseMutex(cache->mutex);
                    return false;
                }
                 memcpy(cache->entries[cache->clock_hand].data, buffer, BLOCK_SIZE); // Копируем новые данные
                cache->entries[cache->clock_hand].ref_bit = true;  // Устанавливаем бит использования
                cout << "Cache block has been rewritten " << cache->clock_hand << endl;
                cache->clock_hand = (cache->clock_hand + 1) % cache->capacity; // перемещаем указатель на следующий элемент
                cout << "Pointer current position: " << cache->clock_hand << endl;
                break;
            }
        }
     }
       ReleaseMutex(cache->mutex);
    return true;
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

typedef struct OpenFileEntry {
    HANDLE file_handle;
    Cache* cache;
    int file_id;
    const char* file_path;
    off_t file_offset;
} OpenFileEntry;

vector<OpenFileEntry> open_files;
int file_id_counter = 0;

int lab2_open(const char* path) {
        // Проверяем, не открыт ли уже этот файл
        for(const auto& file : open_files)
        {
            if (strcmp(file.file_path, path) == 0)
            return file.file_id;
        }

        HANDLE file_handle = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
          if(file_handle == INVALID_HANDLE_VALUE)
          {
            cerr << "Error opening file: " << GetLastError() << endl;
            return -1;
          }

        // Создаем новую запись
        OpenFileEntry new_file;
        new_file.file_handle = file_handle;
        new_file.cache = cache; // Передаем указатель на кэш
        new_file.file_id = file_id_counter++;
        new_file.file_path = path;

        open_files.push_back(new_file);
        return new_file.file_id;

}

int lab2_close(int fd) {
        for (auto it = open_files.begin(); it != open_files.end(); ++it)
        {
            if (it->file_id == fd)
            {
                 if (!CloseHandle(it->file_handle)) {
                     return -1;
                } else {
                    open_files.erase(it);
                    return fd;
                }
             }
        }
        return -1;
}
