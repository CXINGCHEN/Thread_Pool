#include "ThreadPool.h"
#include <chrono>
#include <iostream>

// Task function 1
void taskFunc1(void* arg) {
    int num = *(int*)arg;
    std::cout << "Task " << num << " is processing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Task " << num << " finished processing." << std::endl;
}

// Task function 2
void taskFunc2(void* arg) {
    int num = *(int*)arg;
    std::cout << "Task " << num << " is starting its heavy work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Task " << num << " completed its heavy work." << std::endl;
}

int main() {
    // Create the thread pool
    ThreadPool pool(2, 4); // 2 minimum threads, 4 maximum threads

    // Add 10 tasks to thread pool
    for (int i = 0; i < 10; ++i) {
        int* arg = new int(i);
        if (i % 2 == 0) {
            pool.addTask(Task(taskFunc1, arg));
        }
        else {
            pool.addTask(Task(taskFunc2, arg));
        }
    }

    // Just wait for a while to let tasks execute.
    std::this_thread::sleep_for(std::chrono::seconds(30));

    return 0;
}
