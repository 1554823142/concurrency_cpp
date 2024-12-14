#include"func.h"

/*

*/
class joining_thread 
{ 
  std::thread t; 
public: 
  joining_thread() noexcept=default;            //noexcept: 不会抛出异常
  template<typename Callable,typename ... Args>             //传入可调用对象（如函数或lambda）
  explicit joining_thread(Callable&& func,Args&& ... args): 
    t(std::forward<Callable>(func),std::forward<Args>(args)...)         //通过std::forward完美转发
  {} 
  explicit joining_thread(std::thread t_) noexcept: 
    t(std::move(t_)) 
  {} 
  joining_thread(joining_thread&& other) noexcept: 
    t(std::move(other.t)) 
  {} 
  joining_thread& operator=(joining_thread&& other) noexcept 
  { 
    if(joinable()){ 
      join();                   //它调用 join() 来等待线程结束，避免线程在销毁时未结束
    } 
    t = std::move(other.t); 
    return *this; 
  } 
  joining_thread& operator=(std::thread other) noexcept 
  { 
    if(joinable()) 
      join(); 
    t=std::move(other); 
    return *this; 
  } 
  ~joining_thread() noexcept 
  { 
    if(joinable()) 
    join(); 
  } 
  void swap(joining_thread& other) noexcept 
  { 
    t.swap(other.t); 
  } 
  std::thread::id get_id() const noexcept{ 
    return t.get_id(); 
  } 
  bool joinable() const noexcept 
  { 
    return t.joinable(); 
  } 
  void join() 
  { 
    t.join(); 
  } 
  void detach() 
  { 
    t.detach(); 
  } 
  std::thread& as_thread() noexcept 
  { 
    return t; 
  } 
  const std::thread& as_thread() const noexcept 
  { 
    return t;                   //返回对 std::thread 对象的常量引用
  } 
}; 

void func1()
{
    int i = 40;
    joining_thread jt(do_something, std::ref(i));
    std::cout<< "the joining_thread id is : " << jt.get_id() << std::endl;
    auto new_jt = std::move(jt);
    std::cout<< "the new_joining_thread id is : " << new_jt.get_id() << std::endl;
    std::cout<< "the situation of the joinable is :" << "jt: " << jt.joinable() << "  new_jt: " << new_jt.joinable() <<std::endl;
}

void func2()
{
    int i = 550;
    int j = 600;
    joining_thread jt1(do_something, std::ref(i));
    joining_thread jt2(do_something, std::ref(j));
    std::cout<< "the joining_thread id is : jt1: " << jt1.get_id() << " ,jt2: " << jt2.get_id() << std::endl;
    //测试swap()
    std::cout << "begin swap()" << std::endl;
    jt1.swap(jt2);

    std::cout << "after the swap(),";
    std::cout<< "the joining_thread id is : jt1: " << jt1.get_id() << "  ,jt2: " << jt2.get_id() << std::endl;

}

int main()
{
    //func1();
    func2();
    return 0;
}
