#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

void demo1()
{
    //store()是一个存储操作，而load()是一个加载操作，exchange()是一个“读-改-写”操作
    std::atomic<bool> b;
    bool x=b.load(std::memory_order_acquire);
    b.store(true);                                      //clear() ---> store()写入
    x=b.exchange(false, std::memory_order_acq_rel);     //test_and_change() --> exchange()
}


std::atomic<bool> lock_flag(false);  // 初始化标志为 false，表示锁未被占用

void thread_func(int id) {
    bool expected = false;
    // 自旋锁，试图获取锁
    while (!lock_flag.compare_exchange_weak(expected, true)) {
        // 如果期望值不等于原子变量的值，期望值会被更新为当前的原子变量值
        expected = false;  // 重置期望值
    }

    // 获得锁，进入临界区
    std::cout << "Thread " << id << " has entered the critical section.\n";
    // 模拟临界区的操作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Thread " << id << " is leaving the critical section.\n";

    // 释放锁
    lock_flag.store(false);  // 释放锁，将值设为 false
}

int demo2() {
    std::thread t1(thread_func, 1);
    std::thread t2(thread_func, 2);

    t1.join();
    t2.join();

    return 0;
}

int main()
{
    demo1();
    demo2();
    return 0;
}