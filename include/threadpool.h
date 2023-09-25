#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <future>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <stdexcept>

#define THREADPOOL_MAX_NUM 16

class threadPool {
    inline static threadPool *ptr = nullptr; // 将inline声明于类静态成员变量可以直接定义，无需再到实现文件内定义

    using task = std::function<void()>; // std::function可以包装大多数可调用实体，task为任何无参返回void的函数
    std::vector<std::thread> pool; // 线程池
    std::queue<task> queue; // 任务队列
public:
    inline static std::mutex mtx;
private:
    std::condition_variable cv;

    bool terminate; // 终止标志
public:
    template <typename F, typename ...Args> // 模板参数列表：...Args
    auto push_task(F &&f, Args &&...args) -> std::future<decltype(f(args...))>; // 等价于 std::future<std::result_of<f(args...)>::type>  decltype()和std::result_of<>::type都可以推断类型
    // 需要类型推断时，右值引用&&为万能引用  形如 auto func() -> decltype() 的形式，推断函数返回类型后置，专用于模板元

    static threadPool* getInstance(size_t);
    void destroy() {
        delete this;
    }
private:
    threadPool(size_t);
    ~threadPool();
#ifdef THREAD_AUTO_GROW // 续扩展线程池自动增长
public:
#endif
    void addThread(size_t);
};
// 内敛函数的定义只能和声明在同一个文件内
inline threadPool::threadPool(size_t n = 8) : terminate(false) {
    if (n <= THREADPOOL_MAX_NUM) {
        addThread(n);
    } else {
        throw std::runtime_error("TOO MUCH THREADS");
    }
}

inline threadPool::~threadPool() {
    {
        std::lock_guard grd(mtx);
        terminate = true;
    }
    cv.notify_all(); // 关闭之前唤醒所有被阻塞进程
    for (auto &t : pool)
        if (t.joinable())
            t.join(); // 等线程执行完再退出
}

inline threadPool* threadPool::getInstance(size_t n) {
    {
        std::lock_guard grd(mtx);
        if (ptr == nullptr) {
            try {
                ptr = new threadPool(n);
            }
            catch (std::runtime_error &e) {
                std::cout << e.what() << std::endl;
                return nullptr;
            }
        }
    }
    return ptr;
}

/**
 * @brief threadPool::push_task(lambada)
 * @details 模板参数为一个对象和它的构造函数参数
 * @details 例如有函数print(int)，则实例化push_task(print, int a) -> future<decltype(print(a))>
 * @details 调用形式为push_task([] (int) {...});
*/
template <class F, typename ...Args>
auto threadPool::push_task(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    // std::future可以用来管理异步任务，如get()等待线程执行完获取任务返回值，wait()等待线程执行完毕，通常由std::async(func)或std::promise/std::packaged_task::get_future获得
    using ret_type = decltype(f(args...));
    // std::bind(func, para)将对象与其参数绑定，用std::function<func(para)>保存
    std::function<ret_type()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    // std::packaged_task<func()>包装对象，并且允许异步获取该可调用对象产生的结果，该结果可传递给一个std::future<func>对象
    auto task_ptr = std::make_shared<std::packaged_task<ret_type()>>(func);
    // task_ptr就是个std::packaged_task<func()>类型的智能指针
    {
        std::lock_guard grd(mtx);
        queue.emplace([task_ptr] {
            (*task_ptr)();
        });
    }
    cv.notify_one(); // 唤醒一个线程
    std::future<ret_type> fret = task_ptr->get_future();

    return fret;
}
#endif