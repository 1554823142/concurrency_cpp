#include"func.h"

class scoped_thread 
{ 
  std::thread t; 
public: 
  explicit scoped_thread(std::thread t_): // 1 
    t(std::move(t_)) 
  { 
    if(!t.joinable())  // 2 
      throw std::logic_error("No thread"); 
  } 
  ~scoped_thread() { 
    t.join(); // 3 
  } 
  scoped_thread(scoped_thread const&)=delete; 
  scoped_thread& operator=(scoped_thread const&)=delete; 
}; 
 

 
void f() 
{ 
    int i = 10;
  int some_local_state = 43; 
  //通过使用 {} 初始化列表，编译器能够明确地知道你要传递的是一个**临时对象并进行完美转发**
  //（即通过 std::move 传递），从而避免了对象生命周期和拷贝构造的潜在问题
  //{} 语法提供了更严格的初始化方式，能够避免对象的拷贝，从而通过移动语义来提高效率。
  scoped_thread t({std::thread(func(some_local_state))});    // 4 
  //scoped_thread t(std::thread(func(some_local_state)));
  do_something(std::ref(i)); 
} // 5




int main()
{
  f();
}