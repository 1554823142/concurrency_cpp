#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <algorithm>
#include<stack>

std::list<int> some_list;    // 1 global list
std::mutex some_mutex;       // 2 global mutex

// 向列表添加一个新值
void add_to_list(int new_value)
{
    std::lock_guard<std::mutex> guard(some_mutex);    // 3 数据访问为互斥
    std::cout << "add the number: " << new_value << std::endl;
    some_list.push_back(new_value);
}

// 检查列表中是否包含某个值
bool list_contains(int value_to_find)
{
    std::lock_guard<std::mutex> guard(some_mutex);    // 4 确保访问共享资源时是安全的
    return std::find(some_list.begin(), some_list.end(), value_to_find) != some_list.end();
}

// 模拟线程1，不断往列表中添加元素
void thread_add()
{
    for (int i = 0; i < 10; ++i)
    {
        add_to_list(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟工作负载
    }
}

// 模拟线程2，不断检查列表中是否包含某个值
void thread_check()
{
    for (int i = 0; i < 10; ++i)
    {
        if (list_contains(i))
        {
            std::cout << "Found " << i << " in the list!" << std::endl;
        }
        else
        {
            std::cout << "Did not find " << i << " in the list." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));  // 模拟工作负载
    }
}

int main()
{
    // 启动两个线程
    std::thread t1(thread_add);  // 线程1：添加元素
    std::thread t2(thread_check); // 线程2：检查元素是否存在

    // 等待两个线程结束
    t1.join();
    t2.join();

    return 0;
}
