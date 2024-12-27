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
  
    

- [将异常存与future中](../chap4/exception.cpp)

  函数作为`std::async`的一部分时，当调用抛出一个异常时，这个异常就会存储到`future`中，之后`future`的状态置为“就绪”，之后调用`get()`会抛出已存储的异常(注意：标准级别没有指定重新抛出的这个异常是原始的异常对象，还是一个拷贝。不同的编译器和库将会在这方面做出不同的选择)

  `std::promise`也能提供同样的功能。当存入的是异常而非数值时，就需要调用`set_exception()`成员函数，而非`set_value()`

- [限时等待](../chap4/wait.cpp)

- [使用future的函数化编程](../chap4/FP.cpp)

  **函数化编程**(functional programming)是一种编程方式，函数结果只依赖于传入函数的参数。使用相同的参数调用函数，不管多少次都会获得相同的结果.

  `eg:` 数学公式函数
  
  - ps : `std::list::splice`
  
    用于将一个容器中的元素移动到另一个容器中，或者将某些元素从一个容器中移除并插入到同一容器中的其他位置
  
    `splice()` 函数非常高效，因为它**不会复制元素**，而是直接连接或切断元素的链表结构(即它会**改变原本的调用者**), 并且源列表中的元素将按原顺序插入目标列表。它们在目标列表中**保持相对顺序**
  
    
  
    参数列表:
    
    - `splice` 将一个列表的所有元素插入到另一个列表的某个位置
    
      ​	`void splice (const_iterator pos, list& x);`
    
    - `splice` 将一个列表中的一个元素插入到另一个列表的某个位置
  
      ​	`void splice (const_iterator pos, list& x, const_iterator i);`
    
      ​	**pos**: `x` 列表中元素插入的位置。
    
      ​	**x**: 要被移动的元素的源列表。
    
      ​	**i**: `x` 列表中的元素，将其插入到当前列表的 `pos` 位置。
    
    - `splice` 将一个列表的元素范围插入到另一个列表的某个位置
    
      ​	`void splice (const_iterator pos, list& x, const_iterator first, const_iterator last);`
    
    
    
    
  
- `等待多个future`
  
    假设有很多的数据需要处理，每个数据都可以单独的进行处理, 可以使用异步任务组来处理数据项，每个任务通过future返回处理结果.
  
    但是如果用异步任务来收集结果，先要生成异步任务，这样就会占用线程的资源，并且需要不断的对future进行轮询，当所有future状态为就绪时生成新的任务。逐个 `future` 结果的获取会涉及多次上下文切换和轮询，从而导致资源浪费和性能下降.
  
    ```cpp
    std::future<FinalResult> process_data(std::vector<MyData>& vec) {
        size_t const chunk_size = whatever;
        std::vector<std::future<ChunkResult>> results;
    
        // 将输入数据切分为多个小块，并为每个块启动一个异步任务
        for (auto begin = vec.begin(), end = vec.end(); begin != end;) {
            size_t const remaining_size = end - begin;
            size_t const this_chunk_size = std::min(remaining_size, chunk_size);
    
            // 异步处理数据块
            results.push_back(std::async(process_chunk, begin, begin + this_chunk_size));
            begin += this_chunk_size;
        }
    
        // 创建一个新的异步任务，在所有异步任务完成时收集结果并整合
        return std::async([all_results = std::move(results)]() {
            std::vector<ChunkResult> v;
            v.reserve(all_results.size());
    
            // 等待每个异步任务的结果并收集
            for (auto& f : all_results) {
                v.push_back(f.get());  // 阻塞等待每个 future 完成并获取结果 这就是反复轮询的地方, 频繁的等待和唤醒会带来额外的性能开销，尤其是在大量数据时
            }
    
            // 整合所有结果
            return gather_results(v);
        });
    }
    ```
  
    ​	改进方法:  **使用when_all** 
  
    ​	这个功能允许你等待多个 `future` 完成，而无需逐个调用 `get()`。当所有的 `future` 都完成时，`when_all` 会返回一个新的 `future`，这个 `future` 的状态是就绪的，可以立即用于后续操作
  
    ```cpp
    std::experimental::future<FinalResult> process_data(std::vector<MyData>& vec) {
        size_t const chunk_size = whatever;
        std::vector<std::experimental::future<ChunkResult>> results;
    
        // 将数据切分为多个小块，每个块的大小为 chunk_size，并异步处理每个块
        for (auto begin = vec.begin(), end = vec.end(); begin != end;) {
            size_t const remaining_size = end - begin;
            size_t const this_chunk_size = std::min(remaining_size, chunk_size);
    
            // 异步处理每个数据块
            results.push_back(
                spawn_async(process_chunk, begin, begin + this_chunk_size)
            );
    
            begin += this_chunk_size;  // 移动到下一个数据块
        }
    
        // 使用 when_all 等待所有异步任务完成
        return std::experimental::when_all(results.begin(), results.end()).then( // 1	在所有异步任务完成后，使用 .then() 来定义一个回调函数处理所有的结果
            [](std::future<std::vector<std::experimental::future<ChunkResult>>> ready_results) {
                // 获取所有异步任务的结果
                std::vector<std::experimental::future<ChunkResult>> all_results = ready_results.get();
                
                std::vector<ChunkResult> v;
                v.reserve(all_results.size());
    
                // 获取每个异步任务的结果
                for (auto& f : all_results) {
                    v.push_back(f.get()); // 2
                }
    
                // 整合所有的 ChunkResult 结果
                return gather_results(v);
            }
        );
    }
    
    
    ```
  
    还可以使用`std::experimental::when_any`将`future`收集在一起，当`future`有一个为就绪时，任务即为完成
  
- `锁存器和栅栏`
  
  有时等待的事件是一组线程，或是代码的某个特定点，亦或是协助处理一定量的数据。这种情况下，**最好使用锁存器或栅栏**，而非future
  
  - `std::experimental::latch` 
  
    `latch` 主要用于一种 **倒计时式的等待机制**。它允许线程等待，**直到一个计数器减少到零为止**。通常，`latch` 被用于一种“所有线程都达到某个条件之后才继续”的情形。`latch` 的计数器从一个指定的数值开始，每次调用 `count_down` 操作时，计数器减 1。当计数器的值减到零时，所有等待的线程才会被唤醒。
    
  - `std::experimental::barrier` 
  
    允许一组线程在多个阶段之间进行同步，并在每个阶段结束时确保所有线程都达成一致。每个线程都会在一个特定的同步点上**等待**，直到所有线程都到达该点，才可以继续执行。这种机制特别适用于需要在多个阶段进行同步的任务，例如在并行算法中，需要多个线程在每个阶段结束时等待其他线程。
  
  