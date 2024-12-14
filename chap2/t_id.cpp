#include"func.h"
#include<mutex>
#define NOT_DEFINE
void fun1(std::string str)
{
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << str << " thread id is: " << std::this_thread::get_id() << std::endl;

}


#ifndef NOT_DEFINE
std::thread::id master_thread;

void some_core_part_of_algorithm()
{ 
  if(std::this_thread::get_id()==master_thread) 
  { 
    do_master_thread_work(); 
  } 
  do_common_work(); 
}
#endif

int main()
{
    std::cout << "the main thread id is " << std::this_thread::get_id() << std::endl;
    std::thread t1(pass_int, 10);
    std::cout << "the pass_int thread id is " << t1.get_id() << std::endl;
    t1.join();

    std::thread t2(fun1, "t2");
    std::thread t3(fun1, "t3");
    std::thread t4(fun1, "t4");
    fun1("in the main fun1");
    t2.join();
    t3.join();
    t4.join();
    return 0;
}