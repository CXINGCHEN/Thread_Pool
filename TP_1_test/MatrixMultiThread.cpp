#include <chrono>
//#include "ThreadPool.h"  // Assuming ThreadPool class is in this header
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

class ThreadPool {
public:
    ThreadPool(int numThreads) : stop(false) {
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) {
                        return;
                    }
                    std::function<void()> task(std::move(tasks.front()));
                    tasks.pop();
                    lock.unlock();
                    task();
                }
                });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

    template<typename F, typename... Args>
    void enqueue(F&& f, Args&&... args) {
        std::function<void()> task(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        {
            std::unique_lock<std::mutex> lock(mutex);
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};

using Matrix = std::vector<std::vector<int>>;

// Sequential matrix multiplication
Matrix matrixMultiply(const Matrix& a, const Matrix& b) {
    int n = a.size();
    int m = a[0].size();
    int p = b[0].size();
    Matrix result(n, std::vector<int>(p, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < m; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

// Parallel matrix multiplication using ThreadPool
void matrixMultiplyWorker(const Matrix& a, const Matrix& b, Matrix& result, int startRow, int endRow) {
    int m = a[0].size();
    int p = b[0].size();

    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < m; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

Matrix matrixMultiplyParallel(const Matrix& a, const Matrix& b) {
    int n = a.size();
    int p = b[0].size();
    Matrix result(n, std::vector<int>(p, 0));

    int numThreads = std::thread::hardware_concurrency();
    ThreadPool pool(numThreads);

    int rowsPerThread = n / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int startRow = i * rowsPerThread;
        int endRow = (i == numThreads - 1) ? n : startRow + rowsPerThread;

        pool.enqueue(matrixMultiplyWorker, std::cref(a), std::cref(b), std::ref(result), startRow, endRow);
    }

    return result;
}

// ... [Previous code remains unchanged]

int main() {
    const int SIZE = 100;  // Adjust this for larger matrices
    Matrix a(SIZE, std::vector<int>(SIZE, 2));  // Initialize with value 2 for simplicity
    Matrix b(SIZE, std::vector<int>(SIZE, 3));  // Initialize with value 3 for simplicity

    const int TRIALS = 3;
    double sequentialTime = 0.0, parallelTime = 0.0;

    std::cout << "Sequential multiplication timings (microseconds): ";
    for (int i = 0; i < TRIALS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        Matrix c1 = matrixMultiply(a, b);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        sequentialTime += duration;
        std::cout << duration << " ";
    }
    std::cout << "\nAverage time for sequential multiplication: " << sequentialTime / TRIALS << " microseconds" << std::endl;

    std::cout << "Parallel multiplication timings (microseconds): ";
    for (int i = 0; i < TRIALS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        Matrix c2 = matrixMultiplyParallel(a, b);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        parallelTime += duration;
        std::cout << duration << " ";
    }
    std::cout << "\nAverage time for parallel multiplication: " << parallelTime / TRIALS << " microseconds" << std::endl;

    double improvement = ((sequentialTime - parallelTime) / sequentialTime) * 100;
    std::cout << "Performance improvement with multithreading: " << improvement << "%" << std::endl;

    return 0;
}



