#include <condition_variable>
#include <queue>

#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

// 假设 data_chunk 是一个简单的结构体，表示数据
struct data_chunk {
    int data_id;
    std::string content;

    data_chunk(int id, const std::string& c) : data_id(id), content(c) {}
};

// 用于检查是否还有更多数据要准备
bool more_data_to_prepare() {
    // 假设准备10块数据
    static int count = 0;
    return count < 10;
}

// 准备数据的函数
data_chunk prepare_data() {
    static int count = 0;
    // 模拟准备数据
    count++;
    return data_chunk(count, "Data " + std::to_string(count));
}

// 处理数据的函数
void process(const data_chunk& data) {
    std::cout << "Processing data: " << data.data_id << ", Content: " << data.content << std::endl;
}

// 检查是否是最后一块数据
bool is_last_chunk(const data_chunk& data) {
    return data.data_id == 10;  // 假设最后一块数据的 ID 是 10
}

std::mutex mut;
std::queue<data_chunk> data_queue;  // 1
std::condition_variable data_cond;

void data_preparation_thread() {
    while (more_data_to_prepare()) {
        data_chunk const data = prepare_data();
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);  // 2
        data_cond.notify_one();  // 3
    }
}

void data_processing_thread() {
    while (true) {

        /*
            使用unique_lock的原因;
            std::unique_lock 还支持条件变量（std::condition_variable）的使用，
            允许在 wait() 操作时自动释放锁，等待条件满足后再重新获取锁

            而lock_guard不支持中途unlock()
        */
        std::unique_lock<std::mutex> lk(mut);  // 4 使用unique_lock

        //如果条件不满足,则会释放锁,并阻塞线程;
        //调用notify_one()通知条件变量时，处理数据的线程从睡眠中苏醒，重新获取互斥锁，并且再次进行条件检查
        //在条件满足的情况下，从wait()返回并继续持有锁。当条件不满足时，线程将对互斥量解锁，并重新等待
        data_cond.wait(lk, [] { return !data_queue.empty(); });  // 5 传入锁,以及等待条件(lambda)

        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();  // 6

        process(data);

        if (is_last_chunk(data)) {
            std::cout << "Processing finished." << std::endl;
            break;
        }
    }
}

int main() {
    // 启动准备线程和处理线程
    std::thread preparation_thread(data_preparation_thread);
    std::thread processing_thread(data_processing_thread);

    // 等待线程完成
    preparation_thread.join();
    processing_thread.join();

    return 0;
}

