#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>

using namespace std;

bool is_prime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

void factorize(int num) {
    for (int i = 2; i <= num; ++i) {
        if (is_prime(i) && num % i == 0) {
            cout << i << " ";
            num /= i;
            i = 1;
        }
    }
    if (num > 1) {
        cout << num << " ";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: benchmark1.exe <number of repetitions.>" << endl;
        return 1;
    }

    int repetitions = stoi(argv[1]);
    int num = 85452405;

    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < repetitions; ++i) {
        thread t(factorize, num);
        t.join();
    }
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Time: " << duration << " ms" << endl;

    return 0;
}
