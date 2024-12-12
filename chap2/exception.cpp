#include <thread>
#include <iostream>
int main() {
  std::thread t([] {});
  try {
    throw std::runtime_error("Something went wrong and couldn't be fixed");
  } catch (const std::runtime_error& e) {
    std::cout << t.joinable() << std::endl;
    t.join();  // 处理异常前先 join()
       // 再将异常抛出
  }
  std::cout << bool(t.joinable()) << std::endl;
  try{
    t.join();  
  }
  catch(...){
    std::cout << "rejoin fail" << std::endl;
  }
  
}