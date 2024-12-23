#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <numeric>
#include <windows.h>
#include <filesystem> // for exists

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
            
            char buffer[block_size];
            DWORD bytesRead;
            long long total_bytes_read = 0;
            int hamza = 0;
            
            ULARGE_INTEGER timeStart;
            timeStart.QuadPart = GetTickCount64();
            
            while (ReadFile(hFile, buffer, block_size, &bytesRead, nullptr)) {
                if (bytesRead == 0) break;
            }

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

    return 0;
}
