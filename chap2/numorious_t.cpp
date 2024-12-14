#include "func.h"
#include<vector>


void f()
{ 
  std::vector<std::thread> threads; 
  for (unsigned i = 0; i < 20; ++i) 
  { 
    threads.emplace_back(pass_int, i); // 产生线程 
  }  
  for (auto& entry : threads) // 对每个线程调用 join() 
    entry.join();        
}

int main()
{
    f();
    return 0;
}