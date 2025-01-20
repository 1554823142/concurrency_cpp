#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <chrono>

/// 改进的好处:(change 1~5):
/// push()仅访问tail而不再访问head, 虽然try_pop访问了两者, 但是也仅仅在开始访问tail用作比较, 所以持有锁的时间较短
/// 而dummy node存在的意义在于try_pop()与push()不会再同时访问head和tail-------不需要由全局范围内的互斥锁, 可以各自使用一个互斥锁管理

/// 改进二:上锁(change >= 6)

template<typename T>
class queue {
private:
    struct node {
        // T data;
        std::shared_ptr<T> data;        ////// change 1: 使用指针存储数据
        std::unique_ptr<node> next;
        // node(T data_) : data(std::move(data_)){}
    };
    std::unique_ptr<node> head;
    node *tail;

public:
    queue() : // tail(nullptr){}
                head(new node), tail(head.get()){}      ////// change2: 创建dummy node
    queue(const queue& other)=delete;
    queue& operator=(const queue& other)=delete;
    std::shared_ptr<T> try_pop()
    {
        // if(!head)
        if(head.get() == tail)            ////// change 3：比较head与tail是否相等（因为tail为裸指针， 所以head需要.get()获取裸指针
            return std::shared_ptr<T>();
        std::shared_ptr<T> const res(
                // std::make_shared<T>(std::move(head->data))
                head->data             ////// change 4: get the pointer directly
                );
        std::unique_ptr<node> const old_header = std::move(head);
        head = std::move(old_header->next);
        if(!head)
            tail = nullptr;
        return res;
    }
    void push(T new_value)
    {
        /*std::unique_ptr<node> p(new node(std::move(new_value)));
        node* const new_tail = p.get();
        if(tail)
        {
            tail->next = std::move(p);
        }
        else
        {
            head = std::move(p);
        }*/

        ///// change 5: 先在堆上创建new_value T实例, 然后通过shared_ptr管控它的归属权
        std::shared_ptr<T> new_data(
                std::make_shared<T>(std::move(new_value))
                );
        std::unique_ptr<node> p(new node);      /// 新创建的节点为dummy node 不需要提供new_value
        tail->data = new_data;                  /// 将 new_value 存入原来的 dummy node
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};

template<typename T>
class threadsafe_queue {
private:
    struct node {
        // T data;
        std::shared_ptr<T> data;        ////// change 1: 使用指针存储数据
        std::unique_ptr<node> next;
        // node(T data_) : data(std::move(data_)){}
    };
    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::unique_ptr<node> head;
    node *tail;

    node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);      /// 必须先加head锁, 如果先通过get_tail()获取tail, 会导致后面进行比较时两者的新旧程度不同
        if(head.get() == get_tail())
        {
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
public:
    threadsafe_queue() : // tail(nullptr){}
            head(new node), tail(head.get()){}      ////// change2: 创建dummy node
    threadsafe_queue(const threadsafe_queue& other)=delete;
    threadsafe_queue& operator=(const threadsafe_queue& other)=delete;
    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
    void push(T new_value)
    {
        ///// change 5: 先在堆上创建new_value T实例, 然后通过shared_ptr管控它的归属权
        std::shared_ptr<T> new_data(
                std::make_shared<T>(std::move(new_value))
        );
        std::unique_ptr<node> p(new node);      /// 新创建的节点为dummy node 不需要提供new_value
        node* const new_tail = p.get();
        /// 在此处上锁
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;                  /// 将 new_value 存入原来的 dummy node
        tail->next = std::move(p);
        tail = new_tail;
    }
};

template<typename T>
class threadsafe_queue2 {
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;
public:
    threadsafe_queue2() : head(new node), tail(head.get()) {}    /// 创建dummy node
    threadsafe_queue2(const threadsafe_queue2& other)=delete;
    threadsafe_queue2& operator=(const threadsafe_queue2& other)=delete;

    /// new support
    std::shared_ptr<T> wait_and_pop(){
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }
    void wait_and_pop(T& value){
        std::unique_ptr<node> const old_head = wait_pop_head(value);
    }
    void push(T new_value);

    bool empty(){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return head.get() == get_tail();
    }

    std::shared_ptr<T> try_pop(){
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
    bool try_pop(T& value){
        std::unique_ptr<node> const old_head = try_pop_head(value);
        return old_head;
    }

private:
    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    std::unique_ptr<node> pop_head(){
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
    std::unique_lock<std::mutex> wait_for_data(){
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&]{return head.get() != get_tail();});       /// lambda表达式作为断言
        return std::move(head_lock);            /// 返回锁实例
    }
    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }
    std::unique_ptr<node> wait_pop_head(T& value) {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = std::move(*head->data);
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head(){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get() == get_tail())
            return std::unique_ptr<node>();
        return pop_head();
    }
    std::unique_ptr<node> try_pop_head(T& value){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get() == get_tail())
            return std::unique_ptr<node>();
        value = std::move(*head->data);
        return pop_head();
    }
};

template<typename T>
void threadsafe_queue2<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(
            std::make_shared<T>(std::move(new_value))
            );
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}

void demo1()
{
    threadsafe_queue<int> queue;

    // Start 5 threads that push values into the queue
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread([&queue, i]() {
            queue.push(i);
            std::cout << "Pushed: " << i << std::endl;
        }));
    }

    // Start 5 threads that pop values from the queue
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread([&queue, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Delay to simulate popping after some time
            auto val = queue.try_pop();
            if (val) {
                std::cout << "Popped: " << *val << std::endl;
            } else {
                std::cout << "Pop failed, queue is empty." << std::endl;
            }
        }));
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
}


// 测试用的简单队列操作
void producer(threadsafe_queue2<int>& q) {
    for (int i = 0; i < 10; ++i) {
        std::cout << "Producing: " << i << std::endl;
        q.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟生产过程
    }
}

void consumer(threadsafe_queue2<int>& q) {
    for (int i = 0; i < 10; ++i) {
        auto value = q.wait_and_pop();  // 阻塞直到有数据
        std::cout << "Consuming: " << *value << std::endl;
    }
}


void demo2()
{
    threadsafe_queue2<int> q;  // 创建一个线程安全队列

    // 创建生产者和消费者线程
    std::thread t1(producer, std::ref(q));  // 传入队列的引用
    std::thread t2(consumer, std::ref(q));

    // 等待线程完成
    t1.join();
    t2.join();
}

int main() {

    // demo1();
    demo2();
    return 0;
}