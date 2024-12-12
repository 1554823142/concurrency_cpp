#include <iostream>
#include <thread>
#include"func.h"

struct A {
  void operator()() const { std::cout << 1 << std::endl; }
};

int main() {
  A a;
  std::thread t1(a);  // 会调用 A 的拷贝构造函数
  //std::thread t2(A());  // most vexing parse，声明名为 t2 参数类型为 A 的函数
  std::thread t3{A()};
  std::thread t4((A()));
  std::thread t5{[] { std::cout << 1<< std::endl; }};
  t1.join();
  t3.join();
  t4.join();
  t5.join();
}