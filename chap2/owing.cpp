#include "func.h"

void f()
{
    int i = 10;
    std::thread t1(do_something, std::ref(i));
    std::thread t2 = std::move(t1);         //所有权转移
    t1 = std::thread(mult_param_fun, 30, "I'm a new of thread1");       //移动操作将会隐式的调用
    std::thread t3;
    t3 = std::move(t2);
    //不能通过赋新值给 std::thread 对象的方式来"丢弃"一个线程
    //t1 = std::move(t3); //在此之前，t1已经有了一个关联的线程，这里系统直接调用 std::terminate() 终止程序继续运行

    if(t1.joinable()){
        std::cout << "t1 is joinable" << std::endl;
        t1.join(); 
    }
    if(t2.joinable()){
        std::cout << "t2 is joinable" << std::endl;
        t2.join(); 
    }
    if(t3.joinable()){
        std::cout << "t3 is joinable" << std::endl;
        t3.join(); 
    }
    return;

}

/*
    所有权可以在函数内部转移，std：：thread可以作为参数传递
*/

std::thread func1()
{
    int i = 10;
    return std::thread(do_something, std::ref(i));
}

std::thread func2()
{
    std::thread t(mult_param_fun, 90, "I'm the thread init in func2!");
    return t;
}

void func3(std::thread t){
    if(t.joinable()){
        std::cout << "a thread pass to func3, and it's joining now!" << std::endl;
        t.join();
    }
}

void f2()
{
    auto new_t = func1();
    if(new_t.joinable()){
        new_t.join();
    }
    // auto new_t2 = func2();
    // if(new_t2.joinable())
    //     new_t2.join();
    func3(func2());
    
}

void f3(){
    std::thread t(mult_param_fun, 90, "I'm the thread init in func2!");
    func3(std::move(t));    //当所有权可以在函数内部传递，就允许 std::thread 实例作为参数进行传递
    
}

int main()
{
    //f();
    //f2();
    f3();
    return 0;
}