#include"func.h"

void demo()
{
    std::thread t(mult_param_fun, 3, "hello mult_param!");
    t.join();
}

//如果 指向动态变量的指针作为参数， 传入新的线程中， 原本定义该指针的函数可能在转换为其他类型（如string）前结束
//从而导致未定义行为
void bad_demo()
{
    char buffer[1024]; // 1 
    char ch = 'A';
    int num = 4;
    sprintf(buffer, "the string is %c, and the num is %d", ch, num); 
    std::cout << buffer <<std::endl;
    std::thread t(mult_param_fun, 3, buffer); // 2 
    t.detach();
    //t.join();
}

void demo2()
{
    char buffer[1024]; // 1 
    char ch = 'A';
    int num = 4;
    sprintf(buffer, "the string is %c, and the num is %d", ch, num); 
    std::cout << buffer <<std::endl;
    std::thread t(mult_param_fun, 3, std::string(buffer)); // 使用std::string，避免悬空指针
    t.detach();
    //t.join();
}

int main()
{
    //demo();
    bad_demo();
    return 0;
}