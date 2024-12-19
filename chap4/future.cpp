#include <future>
#include <iostream>
#include <unistd.h>  // sleep 函数需要这个头文件

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

int main()
{
    std::future<int> the_answer = std::async(find_the_answer_ltuae);
    do_other_thing();
    std::cout << "the answer is " << the_answer.get() << std::endl;
}