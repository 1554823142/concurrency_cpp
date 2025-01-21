# 设计基于锁的并发数据结构

- 概念:**串行化（serialization）**

  互斥使多个访问互相排斥：在一个互斥上，每次只可能让一个线程获取锁, 互斥保护数据结构的方式是明令阻止真正的并发访问。

  每个线程轮流访问受互斥保护的数据，它们*只能先后串行依次访问，而非并发访问*

  所以设计的的目标为: **保护的范围越小，需要的串行化操作就越少，并发程度就可能越高**

- 构建安全的数据结构的原则:

  若某线程的行为破坏了数据结构的不变量，则*必须确保其他任何线程都无法见到该状态*

  在数据结构使用的过程中, 限制锁对象的作用域, 尽可能避免嵌套锁, 从而将死锁的可能性降低

## 基于锁的并发数据结构

- [采用锁实现线程安全的栈容器](../chap3/t_stack.cpp)

  每个操作执行前都会加上锁, 虽然安全, 但是锁的排他性使得一次只能有一个线程访问数据, 这样迫使**串行化**

- [采用锁和条件变量实现线程安全的队列容器](../chap6/queue_ts_plus.cpp)

  改进点:将`std::shared_ptr<>`的初始化移动到`push()`处, 令队列存储`std::shared_ptr<>`而不是直接存储数据

  得到的好处是只有在push时才会创建对象, 并且不需要在持有锁的情况下执行复杂的内存分配和对象构造操作(因为创建对象在加锁前), 像这样以安全的方式免除锁的保护, 缩短了互斥锁的持续时长

- [采用精细粒度的锁和条件变量实现线程安全的队列容器]()

  

## 基于锁的并发链表

```cpp
struct node 
 { 
     std::mutex m; 
     std::shared_ptr<T> data; 
     std::unique_ptr<node> next; 
     node(): next() 
     {} 
     node(T const& value): 
     data(std::make_shared<T>(value)) 
     {} 
 };

template<typename Function> 
 void for_each(Function f) 
 { 
     node* current=&head; 
     std::unique_lock<std::mutex> lk(head.m); 
     while(node* const next=current->next.get()) 
     { 
         std::unique_lock<std::mutex> next_lk(next->m); 
         lk.unlock(); 
         f(*next->data); 
         current=next; 
         lk=std::move(next_lk); 
     } 
 }

template<typename Predicate> 
std::shared_ptr<T> find_first_if(Predicate p) 
 { 
     node* current=&head; 
     std::unique_lock<std::mutex> lk(head.m);
     while(node* const next=current->next.get()) 
     { 
         std::unique_lock<std::mutex> next_lk(next->m); 
         lk.unlock(); 
         if(p(*next->data)) 
         { 
             return next->data; 
         } 
         current=next; 
         lk=std::move(next_lk); 
     } 
     return std::shared_ptr<T>(); 
 }

template<typename Predicate> 
 void remove_if(Predicate p) 
 { 
     node* current=&head; 
     std::unique_lock<std::mutex> lk(head.m); 
     while(node* const next=current->next.get()) 
     { 
         std::unique_lock<std::mutex> next_lk(next->m); 
         if(p(*next->data)) 
         { 
             std::unique_ptr<node> old_next=std::move(current->next); 
             current->next=std::move(next->next); 
             next_lk.unlock(); 
         } 
         else 
         { 
             lk.unlock(); 
             current=next; 
             lk=std::move(next_lk); 
         } 
     } 
 }
```

### `for_each`函数

- `template<typename Function> `: 参数模版为一个函数
- 传入的参数`f`为一个函数, 表示对链表的每个元素进行的操作(如打印, 改变数据等)
- 工作流程:
  - 沿着链表前进交替加锁
  - 首先锁住头结点, 然后通过`next`指针的`get()`获得下一节点的指针(因为`next`的类型为`unique_ptr`, 所以不能直接赋值, 否则会直接交换所有权)
  - 然后就可以开始遍历整个链表了, 只要锁住了`next`, 就可以释放当前的节点, 调用`f()`来处理节点
  - 节点处理完成, 就更新节点的信息, 便于继续遍历, 使用`std::move()`来释放`next_lk`, 并对`lk`上锁

### `find_first_if`函数

- 根据`predict`断言来决定何时推出搜索, 其他与`for_each`相同



