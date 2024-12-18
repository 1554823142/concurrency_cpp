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

      