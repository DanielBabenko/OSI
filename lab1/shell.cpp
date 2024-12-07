#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <process.h>
#include <windows.h>

using namespace std;
using namespace chrono;
    
void execute_cd(string path) {
	if (path == "..") {
        SetCurrentDirectory("..");
    } else if (SetCurrentDirectory(path.c_str()) == 0) {
        printf("Error: Cannot change directory to %s\n", path);
    } else {
        SetCurrentDirectory(path.c_str());
    }
} 

void execute_dir() {
    WIN32_FIND_DATA findFileData; 
    HANDLE hFind = FindFirstFile("*", &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error: Unable to list directory.\n");
        return;
    } 

    do {
        printf("%s\n", findFileData.cFileName);
    } while (FindNextFile(hFind, &findFileData) != 0); 
    
    FindClose(hFind);
}
    
void execute_command(string command){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));
    
    HMODULE hKernel32 = GetModuleHandle("Kernel32.dll");
    
    if (!hKernel32) {
        printf("Error: Failed to get handle for Kernel32.dll\n");
        return;
    }
    
    size_t spacePos = command.find(' ');
    string programName = (spacePos == string::npos) ? command : command.substr(0, spacePos);
    string arguments = (spacePos == string::npos) ? "" : command.substr(spacePos + 1);
    
    if (programName == "cd"){
        cout << arguments << endl;
        if (arguments.c_str() != NULL) {
            execute_cd(arguments);
            return;
        } else {
            printf("Usage: cd <directory>\n");
            return;
        }
    } else if (programName == "dir"){
        execute_dir();
        return;
    }

    char cmd[2048] = "";
    snprintf(cmd, sizeof(cmd), "%s", programName.c_str());
    snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %s", arguments.c_str());

    auto startTime = high_resolution_clock::now();
    int result;

        result = CreateProcess(
                programName.c_str(),
                cmd,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &si,
                &pi
        );

    WaitForSingleObject(pi.hProcess, INFINITE);
    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (result == 0) {
        cerr << "Programm launching error: " << GetLastError() << endl;
    } else {
        cout << "Programm completed successfully. Execution time: " << duration.count() << " ms" << endl;
    }
}

int main() {
  string command;

  while (true) {
    cout << ">> ";
    fflush(stdout);
    getline(cin, command);

    if (command == "exit" || command == "quit") {
      break;
    }
    
    execute_command(command);
  }

  return 0;
}
