## 对象内存位置与并发

如果不规定对同一内存地址访问的顺序，那么访问就不是原子的。当两个线程都是“写入者”时，就会产生数据竞争和未定义行为

原子操作作用是当程序对同一内存地址中的数据访问存在竞争，可以使用原子操作来避免未定义行为, 但是原子操作并没有指定访问次序, 只是保证程序不会发生未定义行为

### c++ 原子操作

- **Store 操作**

  `std::atomic::store()` 用于将一个值存储到原子变量中。

- **Load 操作**

  `std::atomic::load()` 用于从原子变量中加载一个值。

- **Read-modify-write 操作**

  原子类型支持的 "读-改-写" 操作包括 `fetch_add()`、`fetch_sub()`、`exchange()` 等。它们在读取原子变量的当前值的同时，进行修改，然后将修改后的结果写回原子变量。

### c++ 内存序

参考:<https://www.zhihu.com/collection/974773714>

> 问题产生的背景:
>
> 处理器读取一个数据时，可能从内存中读取，也可能从缓存中读取，还可能从寄存器读取

C++ 中原子操作的内存序（memory order）决定了操作的顺序性和同步性。不同的内存序在不同的场景下提供不同程度的性能和一致性保证。

常用的内存序:

- **memory_order_relaxed**：不做顺序保证，优化空间最大。
- **memory_order_release**：确保该操作前的所有操作在它之前完成。
- **memory_order_acquire**：确保该操作后的所有操作在它之后完成。
- **memory_order_acq_rel**：同时具有 `release` 和 `acquire` 的特性，适用于同步操作。
- **memory_order_seq_cst**：提供**最强**的内存顺序约束, 强制顺序一致性，确保原子操作的顺序在全局范围内一致。

代表了三种内存模型:

- **[顺序一致性(SEQUENTIALLY CONSISTENT ORDERING)](../chap5/sequentially_consistent.cpp)**	`sequentially consistent`

  若按一定的次序将并行的程序合并为一个串行的程序,则与普通的串行的程序无异

  所有线程看到的原子变量变化是按照某种全序一致的顺序发生, 其他线程会看到一个原子值在另一个线程的更新,

  相比于其他几种内存序, `memory_order_seq_cst`指令执行后则**保证真正写入内存**(没有缓存)

  

  在例子中, z可能为1, 也可能为2, 但是不可能为0, 因为`read_x_then_y`和`read_y_then_x`两个函数分别表示x与y的store先后次序, 如果在一个地方没有实现z++,说明一定已经store了一个值(x或者y), 因为全局都使用了顺序一致, 所以相当于串行程序, 所以一个函数内的z没有增加, 至少说明一个变量已经设置为true了, 则再执行另一个函数时必然使z增加, 若是两个函数都没有使z增加, 就说明这个程序没有体现到**"串行性"**, 也即**单一的全局次序(不同线程所看到的同一组操作的次序和效果一致)**

  

  需要对所有线程进行**全局同步**，所以也是**开销最大**的内存序

  > 若某项操作标记为`memory_order_seq_cst`，则编译器和CPU 须严格遵循源码逻辑流程的先后
  > 顺序。在相同的线程上，以该项操作为界，其后方的任何操作不得重新编排到它前面，而前方的任
  > 何操作不得重新编排到它后面，其中“任何”是指带有任何内存标记的任何变量之上的任何操作

  

- **获取-释放序(ACQUIRE-RELEASE ORDERING)**                 `memory_order_consume`, `memory_order_acquire`, `memory_order_release`和`memory_order_acq_rel`

  虽然操作依旧没有统一顺序，但引入了**同步**

  同步在线程释放和获取间是**成对的(pairwise)**，释放操作与获取操作同步就能读取已写入的值

    - 原子`load`: `acquire`操作(`memory_order_acquire`)	

      保证该操作读取到的值**一定是之前写入**的值(看invalidate queue中是否有该内存的*更新指令*，如果有重新**从主存中加载最新的数据**)

      保证后面其它线程读取该值时**能够读到最新值**，满足**可见性**

    - 原子`store`: `release`操作(`memory_order_release`)

      保证之前的所有**写(也仅对写有限制, 对读没有约束)**操作都在该操作**之前**完成，不能重排序，即保证**happens-before**关系

      保证该操作之前写入的值对其它线程都是可见, 把CPU高速缓存中的数据同步到主存和其它CPU高速缓存中(发送了一个更新指令消息到其它CPU的`invalidate queue`中)

​	*[an example(参考于)](https://www.cnblogs.com/ljmiao/p/18145946)*:

```cpp
#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x_then_y()
{
    x.store(true,std::memory_order_relaxed);// 1
    y.store(true,std::memory_order_release);// 2
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_acquire));// 3
    if(x.load(std::memory_order_relaxed)) //4
        ++z;
}

int main()
{
    x=false;
    y=false;
    z=0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load()!=0);
}
```

- 顺序保证:	

    - 1一定发生于2之前(`happen-before`)

      执行 `y.store(true, std::memory_order_release)`，此操作会确保在它之前对 `x` 的写操作是可见的。这意味着线程 A 对 `x` 的更新（`x = true`）在执行 `y.store` 后是可以被其他线程看到的。

    - 2一定发生于3之前(`Synchronizes-With`)

    		- 3一定发生于4之前(`happen-before`)

  以上的顺序保证`assert`断言始终成立

​	

- **[自由序(RELAXED ORDERING)](../chap5/relaxed.cpp)**                          `memory_order_relaxed`

  唯一的要求是在**同一**线程中，对**同一**原子变量的访问不可以被重排,一旦它见到某原子变量在某时刻持有的值，则该线程的后续读操作不可能读取相对更早的值,  不要求线程间存在任何的次序关系, 不同的原子变量的操作顺序是可以重排的, 线程间仅存的共有信息是**每个变量的改动序列**

  实例中`assert`可能被触发, 因为x和y是两个不同的变量，所以没有顺序去保证每个操作产生相关值的**可见性**
  
  x的store和load分属两个不同的线程, 采用了宽松次序，所以后者不一定能见到前者执行产生的效果，即*存储的新值true 还停留在CPU 缓存中，而读取的false 值是来自内存的旧值*



#### 可见性

指的是一个线程对共享变量的更新，何时能对其他线程可见

多核处理器为了提高效率，会对变量进行**缓存和重排序**。每个线程有可能会在本地缓存中保存变量的副本，而不是直接从主内存中读取。当一个线程修改了某个变量的值，其他线程可能无法立刻看到这个修改(原因在于**缓存的一致性**以及**指令的重排序**)



### 原子操作的约束

- **禁止拷贝和赋值** 

  拷贝构造和拷贝赋值都会将第一个对象的值进行读取，然后再写入另外一个。对于两个独立的对象，这里就有两个独立的操作了，合并这两个操作必定是不原子的。因此，操作就不被允许.

- **不允许默认构造**

  必须显式地对其进行初始化

- **不支持迭代器和比较操作**

  因为比较操作本身可能涉及读取多个线程共享的值，因此需要额外的同步

- **不支持对 `std::atomic` 类型进行指针或引用的操作**

  `std::atomic` 对象的引用只能在某些原子操作的上下文中使用，不能直接被指针操作

- [使用std::atomic_flag实现自旋锁](../chap5/spinlock_mutex.cpp)

- [`std::atomic<bool>`](../chap5/atomic_bool.cpp)

  - **比较交换（Compare-and-Swap, CAS）**

    将一个变量的当前值与期望值进行比较，如果它们相等，则将该变量的值替换为一个新的值。如果它们不相等，则不做任何修改，并且通常会更新期望值为原子变量的当前值。“比较/交换”函数值是一个bool变量，当返回true时执行存储操作，false则更新期望值。

    - 如果它们相等，说明该变量的值没有被其他线程修改过，就将该变量的值替换为新的值。

    - 如果它们不相等，说明有其他线程修改了该变量，操作失败，通常会更新期望值为变量的当前值。

    **c++两种形式的 CAS 操作**:

    - `compare_exchange_weak()`

      **伪失败（spurious failure）**：在某些情况下，`compare_exchange_weak()` 可能会出现“伪失败”，即使原子变量的值与期望值相等，操作也可能会返回 `false`。这通常是**因为底层硬件或操作系统调度**造成的，例如线程调度导致的上下文切换。

      伪失败通常会配合一个**循环**使用

    - `compare_exchange_strong()`
    
    ​    行为与 `compare_exchange_weak()` 类似，但它确保在任何情况下，如果原子变量的值与期望值相等，操作会成功执行并返回 `true`。在某些实现上， 它**可能会避免“伪失败”现象**，因此**通常在没有硬件保障的情况下优先选择使用**。
    

​		与`atomic_flag`不同的是: `std::atomic<bool>`可能不是无锁的, 除了`std::atomic_flag`之外，所有原子类型都拥有的特征(`is_lock_free`)

### 原子操作中的非成员函数

- 大多数非成员函数的命名与对应成员函数有关，需要`atomic_`作为前缀, eg:`atomic_load()`

  `std::atomic_is_lock_free(&a)`返回值与`a.is_lock_free()`相同, 再比如:`std::atomic_load(&a)`和`a.load()`的作用一样

  非成员函数的设计是为了与C语言兼容，因为C语言中没有引用, 而只能使用指针



## 同步操作和强制排序

