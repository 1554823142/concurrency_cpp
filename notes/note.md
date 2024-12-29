- [`启动线程`](../chap2/demo1.cpp)`</br>`
  c++线程库构造 `std::thread`来启动线程，不仅可以传递函数，也可以通过有函数操作符类型的实例来进行构造

  - `detach`与 `join` 的区别 `</br>`

    - `detach`

      detach() 会让线程在 `后台运行`，不再与创建它的线程（主线程）保持关联。调用 detach() 后，`线程会独立执行，主线程不会等待它`。线程会独立执行，且在主线程退出时不会等待它完成。如果线程没有显式结束，detach() 后该线程会成为“分离的”线程，直到它执行完毕或程序结束。希望子线程自动管理其生命周期的情况.如果不正确管理，可能会导致 `资源泄漏`
    - `join`

      join() 会使调用它的线程等待直到被调用的线程执行完成（即该线程终止）。调用 join() 的线程会阻塞，直到目标线程执行完毕。
  - 注意：

    C++thread库的构造函数 `只能`以 `复制的值传递形式`传递参数，因此，以引用形式传递参数会报错，解决办法：在传递的参数前加是 `std::ref()`；

    eg: std::thread t(func, std::ref(param));
- [异常中的join处理](../chap2/exception.cpp)
  在无异常的情况下使用join()时，需要 `在异常处理过程中调用join()`，从而避免生命周期的问题
- [使用RAII形式的类型](../chap2/join_RAII.cpp) `</br>`
  `thread_guard`
- [传递函数参数](../chap2/pass_param.cpp) `</br>`
  尤其注意在传递的参数为引用的时候，在原线程中创建的指针可能会在新线程使用（例如类型转化）之前就释放，所以注意传入之前进行类型转换

  如果期望传递非常量的引用，但复制了整个对象，这时会会出现 `编译错误`,因为构造函数无视函数参数类型，盲目地拷贝已提供的变量。将拷贝的参数以右值的方式进行传递，而函数期望的是一个非常量引用作为参数(而非右值)，所以会在编译时出错
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
  - [线程所有权的转移](../chap2/owing.cpp)
    `std::thread`是可移动的, 通过新线程返回的所有权去调用一个需要后台启动线程的函数

    - ps: `std::terminate() <br>`

      当程序遇到无法处理的异常或者调用了 `std::terminate()` 时，该函数会被调用。它会立即终止程序的执行，并且不会执行任何清理工作（如调用析构函数或释放资源）。`</br>`通常情况下，`std::terminate()` 会调用一个被称为“终止处理函数”（terminate handler）的函数。默认的终止处理函数会调用 `abort()`，这会导致程序异常终止并生成一个核心转储文件（如果系统配置允许的话）。你可以通过调用 `std::set_terminate()` 来设置自定义的终止处理函数。
  - [scoped_thread](../chap2/scope_thread.cpp) `</br>`
  - [joinint_thread](../chap2/joining_thread.cpp) `</br>`
    管理线程生命周期并确保在析构时自动执行 `join` 操作，从而避免线程未被正确结束的情况
  - [量产线程](../chap2/numorious_t.cpp) `</br>`
    注意：多线程可能会打断 `std::cout` 的输出，导致输出内容交错或混乱，`std::cout` 默认 `不是线程安全`的

    - `容器是移动敏感的</br>`
      容器在存储、操作或传递其元素时会 `感知`到元素的移动语义，能够通过 `“移动”`而不是“拷贝”来高效地操作元素

      例如，当我们向容器插入元素时：

      - 如果元素是右值（即临时对象或不再使用的对象），容器将通过移动构造或移动赋值来处理它，从而避免拷贝。
      - 如果元素是左值（即常规的已命名对象），容器将执行传统的拷贝构造或拷贝赋值。
  - [并行版累加](../chap2/accumulate.cpp) `</br>`

    将输入的范围（`first` 到 `last`）`分割成多个块`并在多个线程中并行地执行 `std::accumulate`，然后将每个线程的结果合并成最终的累加结果

    比较了并行运算和串行运算，结果表明：如果数较小（eg:设置的累加的数的总量小于100000个），则串行计算快，但是并行操作在处理大规模的数据计算很快
  - [进程标识](../chap2/t_id.cpp)</br>

    线程标识为 `std::thread::id`类型,可以通过两种类型获得，调用std::thread对象的成员函数`get_id()`来直接获取。如果std::thread对象没有与任何执行线程相关联，get_id()将返回std::thread::type默认构造值，这个值表示“无线程”。
    第二种，当前线程中调用`std::this_thread::get_id()`(这个函数定义在 `<thread>`头文件中)也可以获得线程标识
