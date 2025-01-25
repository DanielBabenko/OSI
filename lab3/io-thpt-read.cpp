// #include <iostream> 
// #include <fstream> 
// #include <chrono> 
// #include <vector> 
// #include <numeric> 
// #include <windows.h> 
// #include <filesystem> // for exists 
 
// using namespace std; 
// using namespace chrono; 
// namespace fs = std::filesystem; 
 
// const long long FILE_SIZE = 1024 * 1024 * 1024; //1GB 
 
 
// auto measure_time(auto start, auto end) { 
//   return chrono::duration_cast<chrono::milliseconds>(end - start).count(); 
// } 
 
// double calculateAverage(const vector<double>& data, int repetitions) { 
//     if (data.empty()) { 
//         return 0.0; 
//     } 
 
//     double sum = accumulate(data.begin(), data.end(), 0.0); 
//     return sum / repetitions; 
// } 
 
// int main(int argc, char* argv[]) { 
//     if (argc != 3) { 
//         cerr << "Usage: benchmark2.exe <number of repetitions> <filename>" << endl; 
//         return 1; 
//     } 
 
//     long long repetitions = stoi(argv[1]); 
//     string filename = argv[2]; 
 
//     if (!fs::exists(filename)) { 
//         cerr << "Error: File '" << filename << "' does not exist." << endl; 
//         return 1; 
//     } 
 
 
//     int block_size = 1024; 
//     vector<double> throughputs(repetitions); 
//     cout << "Block size: " << block_size << " bytes" << endl; 
//     cout << "Reading from file: " << filename << endl; 
     
//     DWORD dwFlags = FILE_FLAG_NO_BUFFERING; 
 
//     auto start = chrono::high_resolution_clock::now(); 
 
//         HMODULE hKernel32 = GetModuleHandle("Kernel32.dll"); 
     
//         if (!hKernel32) { 
//             printf("Error: Failed to get handle for Kernel32.dll\n"); 
//             return 0; 
//         } 
         
//         for (int i = 0; i < repetitions; ++i) { 
//             HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr); 
//             if (hFile == INVALID_HANDLE_VALUE) { 
//                 cerr << "Error opening file: " << GetLastError() << endl; 
//                 return 1; 
//             } 
             
//              LARGE_INTEGER file_size; 
//                 if (!GetFileSizeEx(hFile, &file_size)) { 
//                     cerr << "Error getting file size: " << GetLastError() << endl; 
//                     CloseHandle(hFile); 
//                     return 1; 
//                 } 
             
//             char buffer[block_size]; 
//             DWORD bytesRead; 
//             long long total_bytes_read = 0; 
             
//             ULARGE_INTEGER timeStart; 
//             timeStart.QuadPart = GetTickCount64(); 
             
//             while (ReadFile(hFile, buffer, block_size, &bytesRead, nullptr)) { 
//                 if (bytesRead == 0) break; 
//             } 
 
//             CloseHandle(hFile); 
//             ULARGE_INTEGER timeEnd; 
//             timeEnd.QuadPart = GetTickCount64(); 
             
//             double timeElapsed = (double)(timeEnd.QuadPart - timeStart.QuadPart) / 1000.0; 
//             double throughput = (double)file_size.QuadPart / timeElapsed; 
//             double throughputMB = throughput / (1024.0 * 1024.0); 
 
//             throughputs.push_back(throughputMB); 
//             cout << "Throughput for repetition " << i + 1 << ": " << fixed << setprecision(2) << throughputMB << " MB/s" << endl; 
// } 
//     double averageThroughput = calculateAverage(throughputs, repetitions); 
//     cout << "Average Throughput: " << averageThroughput << " MB/s" << endl; 
     
//     auto end = chrono::high_resolution_clock::now(); 
//     auto duration = measure_time(start, end); 
 
//     cout << "Time: " << duration << " ms" << endl << endl; 
 
//     return 0; 
// }

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <numeric>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath> // For std::ceil
#include <iomanip>
#include <cstring> // For memcpy

using namespace std;
using namespace chrono;

auto measure_time(auto start, auto end) {
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

double calculateAverage(const vector<double>& data, int repetitions) {
    if (data.empty()) {
        return 0.0;
    }

    double sum = accumulate(data.begin(), data.end(), 0.0);
    return sum / repetitions;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: benchmark2 <number of repetitions> <filename>" << endl;
        return 1;
    }

    long long repetitions = stoi(argv[1]);
    string filename = argv[2];


    struct stat file_stat;
    if (stat(filename.c_str(), &file_stat) == -1) {
        cerr << "Error: File '" << filename << "' does not exist." << endl;
        return 1;
    }


     int block_size = 1024;
      vector<double> throughputs(repetitions);
    cout << "Block size: " << block_size << " bytes" << endl;
    cout << "Reading from file: " << filename << endl;


     int fd = -1;
   auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < repetitions; ++i) {
       fd = open(filename.c_str(), O_RDONLY | O_DIRECT);
         if (fd == -1) {
            cerr << "Error opening file: " << strerror(errno) << endl;
            return 1;
         }


         off_t file_size = file_stat.st_size;
          
         char* buffer =  (char*)aligned_alloc(block_size, block_size);

         if (buffer == nullptr) {
            cerr << "Error allocating buffer" << endl;
            close(fd);
            return 1;
         }
         
      long long total_bytes_read = 0;
     struct timespec timeStart, timeEnd;
      clock_gettime(CLOCK_MONOTONIC, &timeStart);

          ssize_t bytesRead;
          while ((bytesRead = read(fd, buffer, block_size)) > 0) {
              total_bytes_read += bytesRead;
          }

        if (bytesRead == -1) {
            cerr << "Error reading file: " << strerror(errno) << endl;
            free(buffer);
             close(fd);
            return 1;
         }

       clock_gettime(CLOCK_MONOTONIC, &timeEnd);

        double timeElapsed = (timeEnd.tv_sec - timeStart.tv_sec) + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1000000000.0;
          double throughput = (double)file_size / timeElapsed;
          double throughputMB = throughput / (1024.0 * 1024.0);

         throughputs.push_back(throughputMB);
        cout << "Throughput for repetition " << i + 1 << ": " << fixed << setprecision(2) << throughputMB << " MB/s" << endl;
    
        free(buffer);
         close(fd);
   }


     double averageThroughput = calculateAverage(throughputs, repetitions);
     cout << "Average Throughput: " << fixed << setprecision(2) << averageThroughput << " MB/s" << endl;


    auto end = chrono::high_resolution_clock::now();
    auto duration = measure_time(start, end);
    cout << "Time: " << duration << " ms" << endl << endl;
     
    return 0;
}