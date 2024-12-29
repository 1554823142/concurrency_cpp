#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

static int num = 0;

//利用std::atomic_flag实现一个简单的自旋锁
/*
std::atomic_flag 是一个 标志位 类型，它的大小通常为一个字节，且只能存储两个值：
false（表示未设置）
true（表示已设置）
*/
class spinlock_mutex
{
    std::atomic_flag flag;
public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT){}
    void lock()
    {   //test_and_set()：原子地检查当前标志位的值，并将其设置为 true。这是一个 原子交换 操作，返回的是标志位的旧值
        while(flag.test_and_set(std::memory_order_acquire));
    }
    void unlock()
    {   //clear()：原子地将标志位设置为 false
        flag.clear(std::memory_order_release);
    }
};
spinlock_mutex m;

void f1()
{
    m.lock();
    for(int i = 0; i < 10000; i++){
        num++;
    }
    m.unlock();
}

int main()
{
    
    std::thread t1(f1);
    std::thread t2(f1);
    std::thread t3(f1);
    std::thread t4(f1);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::cout << "the num is: " << num <<std::endl;
    return 0;
}
