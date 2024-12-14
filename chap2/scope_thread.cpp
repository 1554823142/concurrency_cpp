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
  int some_local_state; 
  scoped_thread t(std::thread(func(some_local_state)));    // 4 
  do_something(std::ref(i)); 
} // 5