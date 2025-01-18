#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <vector>
#include <thread>
#include <iostream>

struct empty_stack: std::exception {
    //what() 函数是 std::exception 类中的虚成员函数，
    //用于提供异常的详细信息。它返回一个指向描述信息的 C 风格字符串（const char*）。
    const char* what() const throw() {          //throw() 说明该函数不会抛出异常
        return "empty stack!";
    }
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;  // 用于存储栈的数据
    mutable std::mutex m; // 互斥量，确保线程安全

public:
    // 默认构造函数
    threadsafe_stack() : data(std::stack<T>()) {}

    // 拷贝构造函数，使用互斥锁确保线程安全
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data; // 1 在构造函数体中的执行拷贝 这样的方式比成员初始化列表好,可以保护
    }

    // 禁止赋值操作
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    // push 操作，向栈中压入一个元素
    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }

    // pop 操作，返回栈顶元素并从栈中移除
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();  // 在调用pop前，检查栈是否为空
        //使用智能指针的原因： 确保栈顶元素在从栈中弹出并返回时不会被销毁,如果使用一般的指针或引用，则返回对象指向一个已销毁的对象
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));  // 在修改堆栈前，分配出返回值
        data.pop();
        return res;         //返回**副本**
    }

    // pop 操作，将栈顶元素复制到传入的引用中，并从栈中移除
    void pop(T& value) {  
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }

    // 检查栈是否为空
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};



void test_threadsafe_stack() {
    // 创建线程安全栈实例
    threadsafe_stack<int> stack;

    // 线程数和操作次数
    const int num_threads = 10;
    const int num_operations = 1000;

    // 向栈中添加元素的线程
    auto push_thread = [&stack]() {
        for (int i = 0; i < num_operations; ++i) {
            stack.push(i);
        }
    };

    // 从栈中弹出元素的线程
    auto pop_thread = [&stack]() {
        for (int i = 0; i < num_operations; ++i) {
            try {
                auto val = stack.pop(); // 使用返回的 shared_ptr
                // std::cout << "Popped value: " << *val << std::endl;
            } catch (const empty_stack& e) {
                // 如果栈为空，可以处理异常
                // std::cout << e.what() << std::endl;
            }
        }
    };

    // 启动多个线程进行压栈操作
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(push_thread)); // 启动 push 线程
    }

    // 启动多个线程进行弹栈操作
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(pop_thread)); // 启动 pop 线程
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 输出测试结果
    std::cout << "Testing completed. Stack operations performed by "
              << num_threads * num_operations << " operations in total." << std::endl;
}

int main() {
    // 测试线程安全栈
    test_threadsafe_stack();
    return 0;
}