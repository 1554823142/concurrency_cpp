#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {
private:
    //因为const 成员函数是不能修改类的任何非 mutable 成员的 如果某个对象声明为 const，
    //那么对它的所有成员函数的调用都不能改变其内部的状态
    mutable std::mutex mut;  // 1 互斥量必须是可变的(支持lock和unlock,解上锁也属于变化)   mutable 允许成员变量即使在常量对象上也能被修改
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}

    threadsafe_queue(threadsafe_queue const& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {                    //一个常量成员函数
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
