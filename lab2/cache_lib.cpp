#include <iostream>
#include <fstream>
#include <windows.h>
#include "cache_lib.h"

using namespace std;

#define BLOCK_SIZE 512

int main() {
    // 1. Инициализация кэша
    const char* disk_file = "disk.bin"; // Имя файла, имитирующего диск
    const size_t cache_capacity = 3;

    // Create dummy disk file
    ofstream disk_stream(disk_file, ios::binary);
    if (disk_stream.is_open()) {
        char buffer[BLOCK_SIZE];
        for (int i = 0; i < 10; i++) {
            sprintf_s(buffer, BLOCK_SIZE, "Hamza # %d", i*i);
           disk_stream.write(buffer, BLOCK_SIZE);
        }
        disk_stream.close();
    }

    if (!cache_init(cache_capacity, disk_file)) {
        cerr << "Error initializing cache." << std::endl;
        return 1;
    }
    
    // 2. Тестирование cache_read
    char buffer[BLOCK_SIZE];
    bool success;

    // Test cache miss (block id = 0)
    success = cache_read(0, buffer);
    if (success) {
      cout << "Read block 0: " << buffer << endl;
    } else {
      cerr << "Error reading block 0." << endl;
    }

    // Test cache hit (block id = 0)
    success = cache_read(0, buffer);
     if (success) {
        cout << "Read block 0 from cache: " << buffer << endl;
    } else {
        cerr << "Error reading block 0." << endl;
    }

    // Test cache miss with replacement (block id 1 and 2)
    success = cache_read(1, buffer);
    if (success) {
        cout << "Read block 1: " << buffer << endl;
    } else {
        cerr << "Error reading block 1" << endl;
    }

    success = cache_read(2, buffer);
    if (success) {
      cout << "Read block 2: " << buffer << endl;
    } else {
      cerr << "Error reading block 2" << endl;
    }

     success = cache_read(3, buffer);
    if (success) {
       cout << "Read block 3: " << buffer << endl;
    } else {
       cerr << "Error reading block 3" << endl;
    }


      success = cache_read(0, buffer);
    if (success) {
      cout << "Read block 0 from cache (again): " << buffer << endl;
    } else {
      cerr << "Error reading block 0" << endl;
    }
    
        // 2. Тестирование cache_write и cache_read (запись нового блока)
    sprintf_s(buffer, BLOCK_SIZE, "New data for block 0");
    success = cache_write(0, buffer); // Записываем новый блок 0
     if (success) {
         cout << "Write block 0 successfully." << endl;
     } else {
         cerr << "Error writing block 0" << endl;
     }


    success = cache_read(0, buffer); // Читаем блок 0 из кэша.
     if (success) {
        cout << "Read block 0 from cache (after write): " << buffer << endl;
    } else {
        cerr << "Error reading block 0" << endl;
    }
    

    // Тестирование промаха и замены
     success = cache_read(1, buffer);
       if (success) {
          cout << "Read block 1: " << buffer << endl;
      } else {
          cerr << "Error reading block 1." << endl;
      }

     success = cache_read(2, buffer);
       if (success) {
          cout << "Read block 2: " << buffer << endl;
       } else {
          cerr << "Error reading block 2" << endl;
       }
        success = cache_read(3, buffer);
     if (success) {
         cout << "Read block 3: " << buffer << endl;
     } else {
         cerr << "Error reading block 3." << endl;
     }

    
    success = cache_write(1, buffer); // Записываем новый блок 1
     if (success) {
         cout << "Write block 1 successfully." << endl;
     } else {
         cerr << "Error writing block 1" << endl;
     }

    success = cache_read(1, buffer);
      if (success) {
        cout << "Read block 1 from cache (after write): " << buffer << endl;
      } else {
        cerr << "Error reading block 1." << endl;
      }
     
     sprintf_s(buffer, BLOCK_SIZE, "PEnskoy Krutoy"); 
     success = cache_write(5, buffer); // Записываем новый блок 1
     if (success) {
         cout << "Write block 5 successfully." << endl;
     } else {
         cerr << "Error writing block 5" << endl;
     }

    success = cache_read(5, buffer);
      if (success) {
        cout << "Read block 5 from cache (after write): " << buffer << endl;
      } else {
        cerr << "Error reading block 5." << endl;
      }

    // 3. Очистка кэша
    cache_destroy();
     
    cout << "Cache destroyed." << endl;

    return 0;
}
