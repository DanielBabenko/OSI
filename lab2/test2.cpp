#include <iostream>
#include <fstream>
#include <windows.h>
#include "cache_lib.h"

using namespace std;

#define BLOCK_SIZE 1024

int test_open_file(const char* disk_file){
    int fd = lab2_open(disk_file);
    if (fd == -1) {
        cerr << "Error opening file." << endl;
         cache_destroy();
        return -1;
    }
    cout << "File opened, fd: " << fd << endl;
    
    return fd;
}

void test_write(int fd, char* write_buffer, size_t length){
     ssize_t bytes_written = lab2_write(fd, write_buffer, length);
     if (bytes_written == -1) {
       cerr << "Error writing to file." << endl;
          lab2_close(fd);
            cache_destroy();
         return;
     }
     cout << "Bytes written: " << bytes_written << endl;
}

void test_read(int fd, char* read_buffer, size_t length){
      ssize_t bytes_read = lab2_read(fd, read_buffer, length);
      if (bytes_read == -1) {
         cerr << "Error reading from file." << endl;
         lab2_close(fd);
         cache_destroy();
         return;
    }
      cout << "Bytes read: " << bytes_read << endl;
      cout << "Data read: ";
      for (size_t i = 0; i < bytes_read; i++) {
        cout << read_buffer[i];
      }
      cout << endl;
}

off_t test_seek(int fd, off_t offset, int whence){
    off_t new_offset = lab2_lseek(fd, offset, whence);
    if (new_offset == -1) {
        cerr << "Error seeking file." << endl;
    } else {
       cout << "New offset: " << new_offset << endl;
    }
    
    return new_offset;
}

int main() {
    // 1. Инициализация кэша
    const char* disk_file = "disk3.bin";
    const size_t cache_capacity = 3;
    std::ofstream disk_stream(disk_file, ios::binary);
    if (disk_stream.is_open()) {
        char buffer[BLOCK_SIZE];
        for (int i = 0; i < 10; i++) {
            sprintf_s(buffer, BLOCK_SIZE, "LOL %d", i);
            disk_stream.write(buffer, BLOCK_SIZE);
        }
        disk_stream.close();
    }


    if (!cache_init(cache_capacity, disk_file)) {
        cerr << "Error initializing cache." << endl;
        return 1;
    }
    
    const char* disk2_file = "disk2.txt";
    const char* disk3_file = "disk3.txt";
    // 2. Тестирование lab2_open
    int fd = test_open_file(disk2_file);
    if (fd == -1) return 1;
    
    fd = test_open_file(disk3_file);
    if (fd == -1) return 1;
    
    fd = test_open_file(disk3_file);
    if (fd == -1) return 1;

    // 3. Тестирование lab2_write
    char write_buffer[] = "Hello, I am Hamza Agaev!";

    test_write(0, write_buffer, strlen(write_buffer));
    test_write(fd, write_buffer, strlen(write_buffer));

    // 4. Тестирование lab2_read
    char read_buffer[100];
    test_read(fd, read_buffer, strlen(write_buffer));
      
    // 5. Тестирование lab2_lseek
    off_t offset = 7;
    off_t new_offset = test_seek(fd, offset, SEEK_SET);

     // 6. Тестирование lab2_read после lab2_lseek
    size_t length = strlen(write_buffer) - new_offset;
    test_read(fd, read_buffer, length); 
        
    // 6,5. Запись после сдвига указателя в файле.
    char write_buffer2[] = "My name is DANIEL BABENKO, not HAMZA!";
    test_write(fd, write_buffer2, strlen(write_buffer2));
    test_read(fd, read_buffer, strlen(write_buffer2));
        
    //6,75. Возвращаем указатель и проверяем чтение.
    new_offset = test_seek(fd, 0, SEEK_SET);
    test_read(fd, read_buffer, strlen(write_buffer2) + offset);
  
    //7. Тест lab2_lseek при whence == SEEK_CUR
    offset = 17;
    new_offset = test_seek(fd, offset, SEEK_CUR);
    new_offset = test_seek(fd, offset, SEEK_CUR);
    length = strlen(write_buffer2) - new_offset + 7;
    test_read(fd, read_buffer, length);

    char write_buffer3[] = "but secretly, I AM HAMZA!!!";
    test_write(fd, write_buffer3, strlen(write_buffer3));
     
    new_offset = test_seek(fd, 0, SEEK_SET);
    length = strlen(write_buffer3) + offset*2;
    test_read(fd, read_buffer, length);
        
    //8. Тест lab2_lseek при whence == SEEK_END
    offset = 1024-53;
    new_offset = test_seek(fd, offset, SEEK_END);
    length = length - (BLOCK_SIZE - offset);
    test_read(fd, read_buffer, length);

    char write_buffer4[] = "LADYBUG!!!";
    test_write(fd, write_buffer4, strlen(write_buffer4));

    new_offset = test_seek(fd, 0, SEEK_SET);
    length = BLOCK_SIZE - offset + strlen(write_buffer4);
    test_read(fd, read_buffer, length);

  // 9. Тестирование lab2_fsync
    if (lab2_fsync(fd) == -1) {
        cerr << "Error synchronizing file." << endl;
        lab2_close(fd);
        cache_destroy();
          return 1;
    } else {
         cout << "File synchronized." << endl;
    }

    // 10. Тестирование lab2_close
    int f2 = lab2_close(fd);
    if (f2 == -1) {
        cerr << "Error closing file." << endl;
        cache_destroy();
        return 1;
    } else {
         cout << "File " << f2 << " closed." << endl;
    }

    // 11. Очистка кэша
      cache_destroy();
      cout << "Cache destroyed." << endl;

    return 0;
}
