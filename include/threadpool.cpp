#include "threadpool.h"

void threadPool::addThread(size_t n = 1) {
    if (n <= THREADPOOL_MAX_NUM) {
        for (int i = 0; i < n; i++) {
            pool.emplace_back(std::thread([this] {
                while (true) {
                    task t;
                    {   // 从任务队列中取出执行
                        std::unique_lock ulc(mtx);
                        cv.wait(ulc, [this] { // 终止运行或队列不为空才被唤醒
                            return terminate || !queue.empty();
                        });
                        if (terminate && queue.empty()) // 终止运行且队列空就直接返回
                            return;
                        t = std::move(queue.front());
                        queue.pop();
                    }
                    t();
                }
            }));
        }
    }
}