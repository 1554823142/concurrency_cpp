#include <thread>
#include <iostream>

void do_something(int& i) {
    std::cout << i << std::endl;
}

class BackgroundTask {
    
public:
int i;
    BackgroundTask(int value) : i(value) {}
    void operator()() {
        do_something(i);
    }
};

// 使用统一的初始化语法
BackgroundTask f(5); // 创建对象并初始化
std::thread my_thread(f);

// 使用lambda表达式，避免了语法解析的问题
std::thread my_thread2([](int i){
    do_something(i);
}, f.i); // 将参数传递给lambda表达式

// 潜在访问隐患修复的代码
struct Func {
    int& i;
    Func(int& i_) : i(i_) {}

    void operator()() {
        for (unsigned j = 0; j < 1000000; ++j) {
            do_something(i);
        }
    }
};

void oops() {
    int some_local_state = 0;
    Func my_func(some_local_state);
    std::thread my_thread(my_func);
    // 确保线程结束后再继续
    my_thread.join(); // 等待线程结束
}

int main() {
    std::cout << "Main begin" << std::endl;
    oops();
    std::cout << "Main end" << std::endl;
}
