#include <iostream>
#include <thread>

class thread_guard 
{ 
  std::thread& t; 
public: 
  explicit thread_guard(std::thread& t_): 
    t(t_) 
  {} 
  ~thread_guard() 
  { 
    if(t.joinable()) // 1 首先判断是否可以join
    { 
      t.join();      // 2 
    } 
  } 
  thread_guard(thread_guard const&)=delete;   // 3 为了不让编译器自动生成。直接对对象进行拷贝或赋值是很危险的，因为这可能会弄丢已汇入的线程
  thread_guard& operator=(thread_guard const&)=delete; 
}; 

void do_something_in_current_thread(){
    std::cout << "do_something_in_current_thread\n";
}
void do_something(int& i) {
    std::cout << i << std::endl;
}


struct func {
    int& i;
    func(int& i_) : i(i_) {}

    void operator()() {
        for (unsigned j = 0; j < 1000000; ++j) {
            do_something(i);
        }
    }
};
 
struct func; // 定义在代码2.1中 
 
void f() 
{ 
  int some_local_state=0; 
  func my_func(some_local_state); 
  std::thread t(my_func); 
  thread_guard g(t); 
  do_something_in_current_thread(); 
}    // 4 

int main()
{
    f();
}