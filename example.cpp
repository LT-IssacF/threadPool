#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "threadpool.h"

int print_with_lock(int i) { // 设置返回值是为了能让std::future调用get()，并无其他用处
    {   // 如果不上锁，打印出来很乱
        std::lock_guard grd(threadPool::mtx);
        std::cout << "task with lock: " << i << std::endl;
    }
    return i;
}

int print_with_no_lock(int i) {
    // 线程对输出流的访问也是异步的，甚至会出现上一个被使用还在输出字符串，下一次使用已经再输出换行符了，线程是并发异步执行的
    std::cout << "task " << "with " << "no " << "lock: " << i << std::endl;
    return i;
}

threadPool *ptr = threadPool::getInstance(16);

int main() {
    std::vector<std::future<int>> tasks;
    for (int i = 0; i < 16; i++) {
        tasks.emplace_back(ptr->push_task(print_with_lock, i));
    }
    for(auto &thread : tasks) {
        thread.get(); // 等每个线程执行完
    }
    tasks.clear();
    std::cout << "------------------" << std::endl;
    for (int i = 0; i < 16; i++) {
        tasks.emplace_back(ptr->push_task(print_with_no_lock, i));
    }
    for(auto &thread : tasks) {
        thread.get();
    }
    ptr->destroy();
    return 0;
}