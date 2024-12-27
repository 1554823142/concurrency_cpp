#include <future>
#include <iostream>
#include <unistd.h>  // sleep 函数需要这个头文件
#include <assert.h>

int find_the_answer_ltuae()
{
    std::cout << "find_the_answer_ltuae :sleep for 3 sec!" << std::endl;
    sleep(3);
    std::cout << "finished sleep 3 sec\n";
    return 3;
}

void do_other_thing()
{
    std::cout << "do other thing :sleep for 4 sec!" << std::endl;
    sleep(4);
    std::cout << "finished sleep 4 sec\n";
}

void f1()
{
    std::future<int> the_answer = std::async(find_the_answer_ltuae);
    do_other_thing();
    std::cout << "the answer is " << the_answer.get() << std::endl;
}

void f2()
{
    std::promise<int> p;
    std::future<int> f(p.get_future());
    assert(f.valid());  // 1 期望值 f 是合法的
    std::shared_future<int> sf(std::move(f));
    assert(!f.valid());  // 2 期望值 f 现在是不合法的
    assert(sf.valid());  // 3 sf 现在是合法的
}

void f3()
{
    std::promise<std::string> p;
    std::shared_future<std::string> sf(p.get_future());  // 1 隐式转移所有权
}


int main()
{
    //f1();
    f2();
    return 0;
}