#include <iostream>
#include <fstream>
#include <chrono>
#include <windows.h>
#include <filesystem> // for exists

using namespace std;
using namespace chrono;
namespace fs = std::filesystem;

const long long FILE_SIZE = 1024 * 1024 * 1024; //1GB


auto measure_time(auto start, auto end) {
  return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: benchmark2.exe <number of repetitions> <filename>" << endl;
        return 1;
    }

    int repetitions = stoi(argv[1]);
    string filename = argv[2];

    if (!fs::exists(filename)) {
        cerr << "Error: File '" << filename << "' does not exist." << endl;
        return 1;
    }


    int block_size = 1024;
    cout << "Block size: " << block_size << " bytes" << endl;
    cout << "Reading from file: " << filename << endl;

    auto start = chrono::high_resolution_clock::now();
//    for (int i = 0; i < repetitions; ++i) {
//        ifstream in(filename, ios::binary);
//        if (!in.is_open()) {
//            cerr << "Error opening file: " << filename << endl;
//            return 1;
//        }
//        char buffer[block_size];
//        in.seekg(0, ios::beg);
//        while (in.read(buffer, block_size)) {
//            cout << "Read: ";
//            for (int j = 0; j < 16 && j < block_size; ++j) {
//                cout << buffer[j];
//            }
//            cout << endl;
//        }
//        in.close();
//    }

        HMODULE hKernel32 = GetModuleHandle("Kernel32.dll");
    
        if (!hKernel32) {
            printf("Error: Failed to get handle for Kernel32.dll\n");
            return 0;
        }
        
        for (int i = 0; i < repetitions; ++i) {
            HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE) {
                cerr << "Error opening file: " << GetLastError() << endl;
                return 1;
            }
            
            char buffer[block_size];
            DWORD bytesRead;
            while (ReadFile(hFile, buffer, block_size, &bytesRead, nullptr)) {
                if (bytesRead == 0) break;
                cout << "Read: ";
                for (int j = 0; j < bytesRead; ++j) {
                    cout << buffer[j];
                }
                cout << endl;

            }

            CloseHandle(hFile);
        }

    auto end = chrono::high_resolution_clock::now();
    auto duration = measure_time(start, end);

    cout << "Time: " << duration << " ms" << endl << endl;

    return 0;
}
