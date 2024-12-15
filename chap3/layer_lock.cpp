#include"chap3.h"

#include<climits>       //ULONG_MAX unsigned long 类型能表示的最大值 32bit: 2^32-1;64bit: 2^64-1

class hierarchical_mutex {
    std::mutex internal_mutex;                              //实际的互斥量锁定机制, 层次值检查、更新都会在它锁定之后进行
    unsigned long const hierarchy_value;                    // 层次值， 决定了锁的顺序
    unsigned long previous_hierarchy_value;                 // 记录当前线程上一个获取的锁的层次值，以便在释放锁时恢复
    static thread_local unsigned long this_thread_hierarchy_value; //存储当前线程的层次值

    // 检查是否违反层次结构 
    //如将一个hierarchical_mutex实例进行上锁，那么只能获取更低层级实例上的锁
    void check_for_hierarchy_violation() {
        if (this_thread_hierarchy_value <= hierarchy_value) {  // 2
            throw std::logic_error("mutex hierarchy violated");
        }
    }

    void update_hierarchy_value() {
        previous_hierarchy_value = this_thread_hierarchy_value;  // 3
        this_thread_hierarchy_value = hierarchy_value;
    }

public:
    explicit hierarchical_mutex(unsigned long value)
        : hierarchy_value(value), previous_hierarchy_value(0) {}

    void lock() {
        check_for_hierarchy_violation();
        internal_mutex.lock();  // 4    获取实际的互斥量锁
        update_hierarchy_value();  // 5 更新当前线程的层次值
    }

    void unlock() {
        if (this_thread_hierarchy_value != hierarchy_value)
            throw std::logic_error("mutex hierarchy violated");  // 9 不是解锁最近上锁的那个互斥量，就需要抛出异常
        this_thread_hierarchy_value = previous_hierarchy_value;  // 6 为当前线程赋予之前的层级值
        internal_mutex.unlock();
    }

    bool try_lock() {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) {  // 7
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

//使用thread_local装饰， 代表当前线程的层级值

//用 thread_local 修饰一个变量时，每个线程都会拥有该变量的独立副本，这意味着不同线程之间互不干扰。
//每个线程会在它的局部地址空间中拥有该变量的一个单独实例，且该实例的生命周期与线程的生命周期一致
thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);  // 8 初始化为最大值， 所以最初所有线程都能被锁住

//----------------------------------------------------------------------------//

hierarchical_mutex high_level_mutex(10000);  // 1
hierarchical_mutex low_level_mutex(5000);    // 2
hierarchical_mutex other_mutex(6000);        // 3

int do_low_level_stuff()
{
    std::cout << "do_low_level_stuff" << std::endl;
    return -1;
}

int low_level_func() {
    std::lock_guard<hierarchical_mutex> lk(low_level_mutex);  // 4
    return do_low_level_stuff();
}

void high_level_stuff(int some_param)
{
    if(some_param == -1)
        std::cout << "high_level_stuff, and get the low level stuff!" << std::endl;
    else    
        std::cout << "high_level_stuff, and NOT get the low level stuff!" << std::endl;
}

void high_level_func() {
    std::lock_guard<hierarchical_mutex> lk(high_level_mutex);  // 6
    high_level_stuff(low_level_func());  // 5
}

void thread_a() {  // 7
    high_level_func();
}

void do_other_stuff()
{
    std::cout << "do_other_stuff" << std::endl;
}

void other_stuff() {
    high_level_func();  // 10 违反了层级结构
    do_other_stuff();
}

void thread_b() {  // 8
    std::lock_guard<hierarchical_mutex> lk(other_mutex);  // 9
    other_stuff();
}


void test_a()
{
    std::thread t1(thread_a);
    std::thread t2(thread_a);

    std::cout << "after finish two thread_a" << std::endl;
    t1.join();
    t2.join();
}

void test_b()
{
    std::thread tb(thread_b);
    tb.join();
}

int main()
{
    //test_a();
    test_b();
    return 0;
}