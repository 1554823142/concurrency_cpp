- [`启动线程`](./chap2/demo1.cpp)</br>
    c++线程库构造`std::thread`来启动线程，不仅可以传递函数，也可以通过有函数操作符类型的实例来进行构造

    - `detach`与`join` 的区别</br>

      - `detach`

        detach() 会让线程在`后台运行`，不再与创建它的线程（主线程）保持关联。调用 detach() 后，`线程会独立执行，主线程不会等待它`。线程会独立执行，且在主线程退出时不会等待它完成。如果线程没有显式结束，detach() 后该线程会成为“分离的”线程，直到它执行完毕或程序结束。希望子线程自动管理其生命周期的情况.如果不正确管理，可能会导致`资源泄漏`
      - `join`

        join() 会使调用它的线程等待直到被调用的线程执行完成（即该线程终止）。调用 join() 的线程会阻塞，直到目标线程执行完毕。

    - 注意：

        C++thread库的构造函数`只能`以`复制的值传递形式`传递参数，因此，以引用形式传递参数会报错，解决办法：在传递的参数前加是`std::ref()`；

        eg: std::thread t(func, std::ref(param));

- [异常中的join处理](./chap2/exception.cpp)
    在无异常的情况下使用join()时，需要`在异常处理过程中调用join()`，从而避免生命周期的问题

- [使用RAII形式的类型](./chap2/join_RAII.cpp)</br>
  `thread_guard`

- [传递函数参数](./chap2/pass_param.cpp)</br>
  尤其注意在传递的参数为引用的时候，在原线程中创建的指针可能会在新线程使用（例如类型转化）之前就释放，所以注意传入之前进行类型转换
  
  如果期望传递非常量的引用，但复制了整个对象，这时会会出现`编译错误`,因为构造函数无视函数参数类型，盲目地拷贝已提供的变量。将拷贝的参数以右值的方式进行传递，而函数期望的是一个非常量引用作为参数(而非右值)，所以会在编译时出错
  这时应该用类似的语法
  ```cpp
  void update_data_for_widget(widget_id w,widget_data& data); // 1
  void oops_again(widget_id w)
  { 
    widget_data data; 
    std::thread t(update_data_for_widget,w,data); // 2 
    display_status(); 
    t.join(); 
    process_widget_data(data); 
  } 

  //使用 std::ref 将参数转换成引用的形
  std::thread t(update_data_for_widget,w,std::ref(data));
  ```

