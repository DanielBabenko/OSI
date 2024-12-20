#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <numeric>
#include <algorithm>
#include <mutex>
#include <math.h>
#include <future>
#include <functional> // std::function

using namespace std;

mutex factors_mutex;

bool is_prime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

// Структура для передачи задач в пул потоков
struct Task {
  int num;
  int start;
  int finish;
  vector<int> factors;
};

void worker_thread(Task& task) {
    int num = task.num;
    int start = task.start;
    int finish = task.finish;
    if (finish == sqrt(num)){
        finish += 1;
    }
    
        for (int j = start; j < finish; ++j) {
            while (num % j == 0 && is_prime(j)) {
                lock_guard<mutex> lock(factors_mutex);
                task.factors.push_back(j);
                num /= j;
            }
        }
};

vector<int> post_factorize (int num, vector<int> factors) {
        for (int f : factors){
            num /= f;
        }
        if (num > 1 && is_prime(num)) {
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
        
        return factors;
}


vector<int> factorize_threaded(int num, int num_threads, vector<thread>& threadPool, vector<future<void>>& results, vector<Task>& tasks) {
    vector<int> factors;
    
    tasks.clear();
    vector<int> ranges(num_threads);
    
    ranges[0] = 2;
    for (int i = 1; i < num_threads; ++i) {
      ranges[i] = sqrt(num) / ((num_threads - i) * (num_threads - i));
    }
    
    sort(ranges.begin(), ranges.end());
    
    for (int i = 0; i < (num_threads-1); ++i) {
      if (ranges[i] < 2 || ranges[i] == ranges[i+1]) continue;
        
      tasks.push_back( {num, ranges[i], ranges[i+1]});
    }
    
    if (tasks.empty()){
       tasks.push_back( {num, 2, num}); 
    }
    
    for (int i = 0; i < tasks.size(); ++i) {
        results[i] = async(launch::async, worker_thread, ref(tasks[i]));
    }
    
    for (int i = 0; i < tasks.size(); ++i) {
        results[i].get();
        factors.insert(factors.end(), tasks[i].factors.begin(), tasks[i].factors.end());
    }
    
    factors = post_factorize(num, factors);
    
    return factors;
}


int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Usage: benchmark1.exe <number of repetitions> <number_to_factorize>" << endl;
    return 1;
  }

  int repetitions = stoi(argv[1]);
  int num_to_factorize = stoi(argv[2]);
  
  if (num_to_factorize < 2) {
    cerr << "ERROR: Numbers to factorize starts with 2, 3, 4... and so on" << endl;
    return 1;
  }

  const int num_threads = thread::hardware_concurrency();
    
  // Пул потоков
  vector<thread> threadPool(num_threads);
  vector<future<void>> results(num_threads);
  vector<Task> tasks; 

  auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < repetitions; ++i) {
        vector<int> factors = factorize_threaded(num_to_factorize, num_threads, threadPool, results, tasks);
        cout << "Factors for " << num_to_factorize << ": ";
        for (int factor : factors) {
          cout << factor << " ";
        }
        cout << endl;
  }

  auto end = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  cout << "Time: " << duration << " ms" << endl;

  return 0;
}
