#include <iostream>
#include <thread>
#include<string>
#include<assert.h>

void do_something(int& i) {
    std::cout << "I'm doing something, like print the int : " << i << std::endl;
}

//传入多个参数的函数
//这些参数会拷贝至新线程的内存空间中(同临时变量一样)。即使函数中的参数是引用的形式，拷贝操作也会执行
void mult_param_fun(int i, std::string const& s)
{
    std::cout << "the int is " << i << ", and the string is " << s << std::endl;
}

typedef struct func{
    int& i;
    func(int& i_) : i(i_) {}


    //仿函数 可以像调用函数一样调用这个对象
    void operator()() {     
        for (unsigned j = 0; j < 10; ++j) {
            do_something(i);
        }
    }
}func;

class BackgroundTask {
public:
int i;
    BackgroundTask(int value) : i(value) {}
    void operator()() {
        do_something(i);
    }
};


