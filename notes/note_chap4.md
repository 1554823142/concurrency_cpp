## 同步操作

- 书上很有意思的例子

  假设你正在一辆在夜间运行的火车上，在夜间如何在正确的站点下车呢？有一种方法是整晚都要醒着，每停一站都能知道，这样就不会错过你要到达的站点，但会很疲倦。另外，可以看一下时间表，估计一下火车到达目的地的时间，然后在一个稍早的时间点上设置闹铃，然后安心的睡会。这个方法听起来也很不错，也没有错过你要下车的站点，但是当火车晚点时，就要被过早的叫醒了。当然，闹钟的电池也可能会没电了，并导致你睡过站。理想的方式是，无论是早或晚，只要当火车到站的时候，有人或其他东西能把你叫醒就好了。这和线程有什么关系呢？当一个线程等待另一个线程完成时，可以持续的检查共享数据标志(用于做保护工作的互斥量)，直到另一线程完成工作时对这个标识进行重置。不过，这种方式会消耗线程的执行时间检查标识，并且当互斥量上锁后，其他线程就没有办法获取锁，就会持续等待。因为对等待线程资源的限制，并且在任务完成时阻碍对标识的设置。类似于保持清醒状态和列车驾驶员聊了一晚上：驾驶员不得不缓慢驾驶，因为你分散了他的注意力，所以火车需要更长的时间，才能到站。同样，等待的线程会等待更长的时间，也会消耗更多的系统资源。

- [条件变量使用](../chap4/cv1.cpp)

  **注意**: 使用wait进行等待条件变量时,传入的锁需要使用`unique_lock`,而不是使用`lock_guard`,因为`unique_lock`可以支持中途解锁,而`lock_guard`不支持

  - `std::condition_variable::wait`: 

    - 释放互斥锁 

      允许其他线程释放条件, 若没释放锁, 那么其他的`notify_one()`, 线程就无法调用, 导致死锁

    - 线程进入阻塞状态

      底层实现通常会将该线程添加到一个等待队列中，等待条件变量的通知。这个等待队列是与条件变量绑定的

    - 被唤醒时重新获得锁

      另一个线程调用 `notify_one()` 或 `notify_all()` 通知等待的线程时，底层实现会从等待队列中选择一个（或多个）线程唤醒, 线程会重新尝试获得它原本持有的互斥锁, **当且仅当获得锁才能执行后续代码**

    - 线程检查条件

      `wait()` 会在条件满足后唤醒线程，但在唤醒后，线程并不会立即继续执行; 因为其他线程可能在唤醒时已经*修改*了共享数据，导致条件变量的条件并不满足; 因此，**线程通常需要在被唤醒后再次检查条件是否已经满足**

    - 循环等待

      等待可能会被**虚假唤醒**(即未调用`notify_one()`就被唤醒), 使用循环包围`wait()`即可解决, 会重新检查条件，确保在条件满足时才会继续执行

    - 一个简单的伪代码实例

      ```cpp
      void condition_variable::wait(std::unique_lock<std::mutex>& lock) {
          // 1. 在进入等待状态之前释放互斥锁
          lock.release();
      
          // 2. 将当前线程添加到等待队列
          wait_queue.push(current_thread);
      
          // 3. 挂起当前线程并等待其他线程的通知
          block(current_thread);
      
          // 4. 被唤醒后，重新获得互斥锁
          lock.acquire();
      
          // 5. 继续执行
      }
      ```
  
- [future使用](../chap4/future.cpp)
  
  - 假设有一个需要长时间的运算，需要计算出一个有效值，但并不迫切需要这个值。你可以启动新线程来执行这个计算，你需要计算的结果，而`std::thread`并不提供直接接收返回值的机制。这里就需要`std::async`函数模板
  
    使用`std::async`启动一个**异步任务**, 返回一个`std::future`对象, 这个对象持有最终计算出来的结果; 当需要这个值时，只需要调用这个对象的`get()`成员函数，就会阻塞线程直到future为就绪为止，并返回计算结果
  
  - 上面写的例子中, 打印的结果为
  
    ```
    do other thing :sleep for 4 sec!
    find_the_answer_ltuae :sleep for 3 sec!
    finished sleep 3 sec
    finished sleep 4 sec
    the answer is 3
    ```
  
    并且前两行同时打印, 说明`std::async`开启了一个**异步线程**, 并且它执行完毕后打印结束
  
- [后台任务的返回值](../chap4/future_param.cpp)

  `std::async` : 允许向函数传递参数 . 第一个参数是指向成员函数的指针，第二个参数提供这个函数成员类的具体对象(是通过指针，也可以包装在`std::ref`中)，剩余的参数可作为函数的参数传入。

  ```cpp	
  std::future<R> std::async(
      std::launch policy,  // 启动策略，指定是否异步执行
      Callable&& f,        // 可调用对象：可以是普通函数、函数指针、Lambda 表达式、成员函数、函数对象等
      Args&&... args       // 调用 `f` 时传递的参数
  );
  
  //使用成员函数和对象指针
  struct X {
      void foo(int a, std::string const& s) {
          std::cout << "foo called with: " << a << ", " << s << std::endl;
      }
  };
  
  int main() {
      X x;
      std::future<void> f3 = std::async(&X::foo, &x, 42, "hello");
      f3.get();  // 调用对象 x 的成员函数 foo
      return 0;
  }
  ```

  
  
  - [future与任务关联](../chap4/package_task.cpp)
  
    `std::packaged_task` 是 C++11 中引入的一个模板类，它封装了一个**可调用对象**(某个任务)（如函数、函数指针、lambda 表达式或成员函数），并将其与一个 `std::future` 对象绑定。它允许在异步执行的过程中传递结果，并且可以与线程一起使用。即使用`package_task`对任务进行打包, 使用`future`机制获得异步的结果.
  
    - `std::packaged_task` 本身并不会直接执行任务，而是提供了一个接口 `operator()` 来触发任务的执行。
  
    - `std::future` 可以在任务完成时获取返回值或异常。
  
  - `使用std::promise`
  
    `std::promise/std::future`对提供一种机制：`future`可以阻塞等待线程，提供数据的线程可以使用`promise`对相关值进行设置，并将`future`的状态置为“就绪”. 即一个线程能够设置一个值（通常是计算结果），而另一个线程则能够获取该值
  
    也是使用`get_future()`函数获取`std::future`对象, 在`promise`设置完毕, 则设置`future`为就绪, 可以实现跨线程的通信
  
    ```cpp
    #include <future>
    
    void process_connections(connection_set& connections) {
        while (!done(connections)) { // 1
            for (connection_iterator connection = connections.begin(), end = connections.end(); 
                 connection != end; 
                 ++connection) // 遍历多个连接对象
            { 
                if (connection->has_incoming_data()) { // 3 如果该连接有未处理的入站数据
                    data_packet data = connection->incoming();
                    std::promise<payload_type>& p = connection->get_promise(data.id); // 4 用于异步任务的结果传递
                    p.set_value(data.payload);	// 使用std::promise设置该数据的负载值
                }	/* 当数据包成功到达并准备好时，通知与该数据包相关的线程或者调用者，数据已经准备好了。其他线程（例如等待该数据的线程）可以通过 std::future 来获取这个结果。*/
    
                if (connection->has_outgoing_data()) { // 5 如果该连接有待发送的出站数据
                    outgoing_packet data = connection->top_of_outgoing_queue();
                    connection->send(data.payload); // 发送数据包的负载（data.payload）到远程连接
                    data.promise.set_value(true); // 6 表明数据已成功发送
                }
            }
        }
    }
    ```
  
    - 另一个简单的future与promise的例子
  
    ```cpp
    #include <iostream>
    #include <thread>
    #include <future>
    
    void calculate_square(int x, std::promise<int> p) {
        int result = x * x;
        p.set_value(result);  // 设置计算结果
    }
    
    int main() {
        std::promise<int> p;          // 创建一个promise对象
        std::future<int> f = p.get_future();  // 获取与promise关联的future
    
        std::thread t(calculate_square, 10, std::move(p));  // 启动线程并传递promise
    
        // 线程执行完成后获取结果
        std::cout << "The square is: " << f.get() << std::endl;  // 输出结果
    
        t.join();  // 等待线程完成
        return 0;
    }
    
    ```
  
    