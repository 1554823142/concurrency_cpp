#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

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


int main() {
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

    return 0;
}