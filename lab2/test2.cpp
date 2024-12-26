#include <iostream>
#include <fstream>
#include <windows.h>
#include "cache_lib.h"

using namespace std;

#define BLOCK_SIZE 512

int main() {
    // 1. Инициализация кэша
    const char* disk_file = "disk.bin";
    const size_t cache_capacity = 3;
    std::ofstream disk_stream(disk_file, std::ios::binary);
    if (disk_stream.is_open()) {
        char buffer[BLOCK_SIZE];
        for (int i = 0; i < 10; i++) {
            sprintf_s(buffer, BLOCK_SIZE, "Data for block %d", i);
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
    int fd = lab2_open(disk2_file);
    if (fd == -1) {
        cerr << "Error opening file." << endl;
         cache_destroy();
        return 1;
    }
    cout << "File opened, fd: " << fd << endl;
    
    fd = lab2_open(disk3_file);
    if (fd == -1) {
        cerr << "Error opening file." << endl;
         cache_destroy();
        return 1;
    }
    cout << "File opened, fd: " << fd << endl;
    
    fd = lab2_open(disk3_file);
    if (fd == -1) {
        cerr << "Error opening file." << endl;
         cache_destroy();
        return 1;
    }
    cout << "File opened, fd: " << fd << endl;


    // 3. Тестирование lab2_write
    char write_buffer[] = "Hello, I am Hamza Agaev!";
    ssize_t bytes_written = lab2_write(fd, write_buffer, strlen(write_buffer));
     if (bytes_written == -1) {
       cerr << "Error writing to file." << std::endl;
          lab2_close(fd);
            cache_destroy();
         return 1;
     }
     cout << "Bytes written: " << bytes_written << endl;

    bytes_written = lab2_write(fd, write_buffer, strlen(write_buffer));
     if (bytes_written == -1) {
       cerr << "Error writing to file." << std::endl;
          lab2_close(fd);
            cache_destroy();
         return 1;
     }
     cout << "Bytes written: " << bytes_written << endl;

    char write_buffer2[] = "DANIEL BABENKO IS THE BEST!";
    bytes_written = lab2_write(0, write_buffer2, strlen(write_buffer2));
     if (bytes_written == -1) {
       cerr << "Error writing to file." << std::endl;
          lab2_close(fd);
            cache_destroy();
         return 1;
     }
     cout << "Bytes written: " << bytes_written << endl;

    // 4. Тестирование lab2_read
    char read_buffer[100];
    ssize_t bytes_read = lab2_read(fd, read_buffer, strlen(write_buffer));
      if (bytes_read == -1) {
         cerr << "Error reading from file." << endl;
         lab2_close(fd);
         cache_destroy();
         return 1;
    }
      cout << "Bytes read: " << bytes_read << endl;
    
      cout << "Data read: " << read_buffer << endl;
      

    // 5. Тестирование lab2_lseek
    off_t new_offset = lab2_lseek(fd, 0, 0);
    if (new_offset == -1) {
        cerr << "Error seeking file." << endl;
    } else {
       cout << "New offset: " << new_offset << endl;
    }

     // 6. Тестирование lab2_read после lab2_lseek
    bytes_read = lab2_read(fd, read_buffer, strlen(write_buffer));
    if (bytes_read == -1) {
        cerr << "Error reading from file after lseek." << endl;
         lab2_close(fd);
        cache_destroy();
          return 1;
    }
        cout << "Bytes read after seek: " << bytes_read << endl;
        cout << "Data read after seek: " << read_buffer << endl;
        
        bytes_read = lab2_read(fd, read_buffer, strlen(write_buffer));
    if (bytes_read == -1) {
        cerr << "Error reading from file after lseek." << endl;
         lab2_close(fd);
        cache_destroy();
          return 1;
    }
        cout << "Bytes read after seek: " << bytes_read << endl;
        cout << "Data read after seek: " << read_buffer << endl;
        
        bytes_read = lab2_read(fd, read_buffer, strlen(write_buffer));
    if (bytes_read == -1) {
        cerr << "Error reading from file after lseek." << endl;
         lab2_close(fd);
        cache_destroy();
          return 1;
    }
        cout << "Bytes read after seek: " << bytes_read << endl;
        cout << "Data read after seek: " << read_buffer << endl;

  // 7. Тестирование lab2_fsync
    if (lab2_fsync(fd) == -1) {
        cerr << "Error synchronizing file." << endl;
        lab2_close(fd);
        cache_destroy();
          return 1;
    } else {
         cout << "File synchronized." << endl;
    }

    // 8. Тестирование lab2_close
    int f2 = lab2_close(fd);
    if (f2 == -1) {
        cerr << "Error closing file." << endl;
        cache_destroy();
        return 1;
    } else {
         cout << "File " << f2 << " closed." << endl;
    }

    // 9. Очистка кэша
      cache_destroy();
      cout << "Cache destroyed." << endl;

    return 0;
}
