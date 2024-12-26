#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
#include <iostream>

std::mutex m;
std::deque<std::packaged_task<void()>> tasks;

// 模拟接收到 GUI 关闭消息
bool gui_shutdown_message_received() {
    // 在实际代码中，这里应该有一些判断逻辑
    return false; // 假设还没有接收到关闭消息
}

// 模拟获取并处理 GUI 消息
void get_and_process_gui_message() {
    // 在实际代码中，这里应该有消息处理逻辑
    //用户点击和执行在队列中的任务
}

// GUI 线程函数
void gui_thread() { // 1
    while (!gui_shutdown_message_received()) { // 2
        get_and_process_gui_message(); // 3
        
        std::packaged_task<void()> task;
        
        {
            std::lock_guard<std::mutex> lk(m);
            if (tasks.empty()) // 4
                continue;

            task = std::move(tasks.front()); // 5   提取队列中的一个任务 
            tasks.pop_front();
        }   //获得任务后, 由于出代码块, 所以lock_guard自动释放锁
        
        task(); // 6 执行任务, 执行完毕后 ,future为就绪
    }
}

// 创建并启动 GUI 线程
std::thread gui_bg_thread(gui_thread);

// 将任务添加到 GUI 线程的队列中
//将自定义的任务(例子中是打印一条的lambda表达式 )
template <typename Func>
std::future<void> post_task_for_gui_thread(Func f) {
    std::packaged_task<void()> task(f); // 7    可以提供一个打包好的任务 f封装为一个 std::packaged_task<void()> 对象
    std::future<void> res = task.get_future(); // 8

    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task)); // 9

    return res; // 10
}

int main() {
    // 测试：将任务提交到 GUI 线程
    auto future = post_task_for_gui_thread([]() {
        // 这里是你希望在 GUI 线程中执行的任务
        std::cout << "Task executed in GUI thread!" << std::endl;
    });

    // 等待任务完成
    future.get();

    // 停止 GUI 线程（实际情况中，可能需要更复杂的停止机制）
    return 0;
}
