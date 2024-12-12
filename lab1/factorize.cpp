#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <numeric>
#include <algorithm>
#include <mutex>
#include <math.h>

using namespace std;

mutex factors_mutex;

bool is_prime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

vector<int> factorize_threaded(int num, int num_threads) {
    vector<int> factors;
    vector<thread> threads;
    vector<int> ranges(num_threads);
    
    int chunk_size = (sqrt(num) / num_threads) + 1;
    //По-хорошему бы распределить равномерно, но для больших чисел батрачит только 1 поток
//    for (int i = 0; i < num_threads; ++i) {
//      ranges[i] = (i == num_threads - 1) ? sqrt(num) + 1 : i * chunk_size + 2;
//    }
    
    ranges[0] = 2;
    for (int i = 1; i < num_threads; ++i) {
      ranges[i] = sqrt(num) / ((num_threads - i) * (num_threads - i));
    }
    //я понимаю, что такая вещь тоже не так эффективна, но очевидно, что чем дальше в лес,
    //тем труднее найти простое число.
    //Так хотя бы большая часть потоков делает хоть что-то.

    for (int i = 0; i < (num_threads-1); ++i) {
      int finish_thread = ranges[i+1];
      threads.emplace_back([&](int start){
            for (int j = start; j <= finish_thread; ++j) {
                while (num % j == 0 && is_prime(j)) {
                    lock_guard<mutex> lock(factors_mutex);
                    factors.push_back(j);
                    num /= j;
                }
            }
            if (num > 1 && is_prime(num)) {
                lock_guard<mutex> lock(factors_mutex);
                bool already_exists = false;
                for (int f : factors)
                {
                  if (f == num)
                  {
                    already_exists = true;
                    break;
                  }
                }
                if (!already_exists) factors.push_back(num);
            }
            return;
      }, ranges[i]);
    }
    

    for (auto& thread : threads) {
        thread.join();
    }
    
    return factors;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: benchmark1.exe <number of repetitions> <number_to_factorize>" << endl;
        return 1;
    }

    int repetitions = stoi(argv[1]);
    int num_to_factorize = stoi(argv[2]);
    
    const int num_threads = thread::hardware_concurrency();
  
    auto start = chrono::high_resolution_clock::now();
    
    for (int i = 0; i < repetitions; ++i) {
        vector<int> factors = factorize_threaded(num_to_factorize, num_threads);
        cout << "Factors for " << num_to_factorize << ": ";
        for (int factor : factors) {
            cout << factor << " ";
        }
        cout << endl;
        
        //Эта часть была нужна, чтобы проверить, что процесс идёт и всё ок.
        //Но т.к. Хамза крут, я её оставлю.
        if (i % 52 == 0){
            cout << "I am Hamza. Hello!" << endl;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Time: " << duration << " ms" << endl;

    return 0;
}
