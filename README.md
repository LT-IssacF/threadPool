#### 用单例模式实现了一个具备基本功能的线程池 ####
在[这里](https://github.com/progschj/ThreadPool)，[这里](https://github.com/lzpong/threadpool)以及[其他地方](https://zhuanlan.zhihu.com/p/636156144)学了很多
最核心的两个函数：**push_task()**和**add_thread()**
## push_task() ##
使用模板参数列表、函数返回类型后置和自动推断类型等技巧，使用户可以使用线程池处理自己的`task()`
#### 如何编写使用自己的`task()` ####
推荐使用STL容器处理多个任务，类型为`std::future<ret_type>`，就比如下面这样:

    std::vector<std::future<int>> tasks;
然后将自己的任务函数`emplace_back()`进去
#### 参数 ####
    threadPool::push_task(func_name, func_para, func_para2, ...)
推荐使用带返回值的任务函数，这样可以通过在主函数返回前对容器内的`future`的`get()`操作，使得在每个线程在执行完后才会退出，当然也可以不设置返回值，但这样就只有用`std::this_thread::sleep_for()`来达到上述目的了
## add_thread() ##
本线程池设置最大线程数量为**16**，超过会报错
`threadPool`对象内置`std::vector`来存储线程，当有任务时从任务队列中取出并执行，没任务就使用条件变量`std::condition_variable`阻塞`wait()`，直到停止运行或有任务
