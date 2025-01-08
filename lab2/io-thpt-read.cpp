#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <numeric>
#include <windows.h>
#include <filesystem> // for exists
#include "cache_lib.h"

using namespace std;
using namespace chrono;
namespace fs = std::filesystem;

const long long FILE_SIZE = 1024 * 1024 * 1024; //1GB


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
        cerr << "Usage: benchmark2.exe <number of repetitions> <filename>" << endl;
        return 1;
    }

    long long repetitions = stoi(argv[1]);
    string filename = argv[2];

    if (!fs::exists(filename)) {
        cerr << "Error: File '" << filename << "' does not exist." << endl;
        return 1;
    }


    int block_size = 1024;
    vector<double> throughputs(repetitions);
    cout << "Block size: " << block_size << " bytes" << endl;
    cout << "Reading from file: " << filename << endl;

    DWORD dwFlags = FILE_FLAG_NO_BUFFERING;

    auto start = chrono::high_resolution_clock::now();

        HMODULE hKernel32 = GetModuleHandle("Kernel32.dll");

        if (!hKernel32) {
            printf("Error: Failed to get handle for Kernel32.dll\n");
            return 0;
        }
        
        const char* disk_file = "disk.bin";
        
        ifstream infile;
        ofstream outfile;

        infile.open(filename.c_str());
        outfile.open(disk_file);

        char bufferr[block_size];

        while(!infile.eof())
        {
            infile.getline(bufferr,sizeof(bufferr));
            outfile<<bufferr<<endl;
        }

        infile.close();
        outfile.close();
        
        //const size_t cache_capacity = 417961;
        const size_t cache_capacity = 700;
        //const size_t cache_capacity = 100;

        if (!cache_init(cache_capacity, disk_file)) {
            cerr << "Error initializing cache." << endl;
            return 1;
        }

        for (int i = 0; i < repetitions; ++i) {
            HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE) {
                cerr << "Error opening file: " << GetLastError() << endl;
                return 1;
            }

                LARGE_INTEGER file_size;
                if (!GetFileSizeEx(hFile, &file_size)) {
                    cerr << "Error getting file size: " << GetLastError() << endl;
                    CloseHandle(hFile);
                    return 1;
                }
            CloseHandle(hFile);
            
            int fd = lab2_open(filename.c_str());
                if (fd == -1) {
                    cerr << "Error opening file." << endl;
                    cache_destroy();
                    return 1;
                }
            //cout << "File opened, fd: " << fd << endl;

            char buffer[block_size];
            DWORD bytesRead;

            ULARGE_INTEGER timeStart;
            timeStart.QuadPart = GetTickCount64();
            ssize_t bytes_read = 0;
            ssize_t bytes_written = 0;
            int tmp_file_size = (int)file_size.QuadPart;
            long long total_bytes_read = 0;

            while(tmp_file_size > 0) {
                  int bytesToRead = min(tmp_file_size, block_size);
                  bytes_read = lab2_read(fd, buffer, bytesToRead);
                  //cout << "Bytes read: " << bytes_read << endl;
//                  cout << "Data read: " << endl;
//                  for (size_t i = 0; i < bytes_read; i++) {
//                        cout << buffer[i];
//                    }   
//                  cout << " " << endl;
                  total_bytes_read += bytes_read;
                  tmp_file_size -= bytes_read;
                }

                int f2 = lab2_close(fd);
                if (f2 == -1) {
                    cerr << "Error closing file." << endl;
                    cache_destroy();
                    return 1;
                } 
//                else {
//                     cout << "File " << f2 << " closed." << endl;
//                }

            CloseHandle(hFile);
            ULARGE_INTEGER timeEnd;
            timeEnd.QuadPart = GetTickCount64();

            double timeElapsed = (double)(timeEnd.QuadPart - timeStart.QuadPart) / 1000.0;
            double throughput = (double)file_size.QuadPart / timeElapsed;
            double throughputMB = throughput / (1024.0 * 1024.0);

            throughputs.push_back(throughputMB);
            cout << "Throughput for repetition " << i + 1 << ": " << fixed << setprecision(2) << throughputMB << " MB/s" << endl;
}
    
    double averageThroughput = calculateAverage(throughputs, repetitions);
    cout << "Average Throughput: " << averageThroughput << " MB/s" << endl;

    auto end = chrono::high_resolution_clock::now();
    auto duration = measure_time(start, end);

    cout << "Time: " << duration << " ms" << endl << endl;
    
    cache_destroy();
    //cout << "Cache destroyed." << endl;

    return 0;
}