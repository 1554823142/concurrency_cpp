#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>

void system_clock()
{
    using namespace std::chrono;
    system_clock::time_point now = system_clock::now();  //system_clock 代表系统时钟，返回的是当前实际时间
    
    // 将当前时间点转换为time_t类型
    std::time_t now_c = system_clock::to_time_t(now);
    
    std::cout << "Current time: " << std::ctime(&now_c);
}

void steady_clock()     //时间是单调递增的，不受系统时间调整（例如夏令时切换、系统时间同步等）影响。它主要用于度量时间间隔
{
    using namespace std::chrono;
    
    steady_clock::time_point start = steady_clock::now();
    
    // 模拟一些耗时操作
    std::this_thread::sleep_for(seconds(2));
    
    steady_clock::time_point end = steady_clock::now();
    duration<double> elapsed = duration_cast<duration<double>>(end - start);
    
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}

void high_resolution_clock()    //提供的时钟精度通常是最高的，适合需要高精度计时的场景，例如性能测试
{
    using namespace std::chrono;
    
    high_resolution_clock::time_point start = high_resolution_clock::now();
    
    // 模拟一些耗时操作
    std::this_thread::sleep_for(milliseconds(10));
    
    high_resolution_clock::time_point end = high_resolution_clock::now();
    duration<double> elapsed = duration_cast<duration<double>>(end - start);
    
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}

/*
    std::chrono::duration 是用来表示时间间隔的模板类。它有两个模板参数：

第一个参数指定时间间隔的类型（如 int, long, double 等）。
第二个参数是时间单位，使用 std::ratio 来表示时间的分子和分母。
例如，std::chrono::duration<int, std::ratio<60, 1>> 表示以秒为单位的分钟。
预定义的时间段类型包括：

std::chrono::nanoseconds：纳秒
std::chrono::microseconds：微秒
std::chrono::milliseconds：毫秒
std::chrono::seconds：秒
std::chrono::minutes：分钟
std::chrono::hours：小时

*/

void duration_test()        //count() 获取时间的数值部分
{
    using namespace std::chrono;

    // 创建一个表示1小时的时间段
    hours one_hour(1);
    std::cout << "One hour is " << one_hour.count() << " hours." << std::endl;
    
    // 创建一个表示500毫秒的时间段
    milliseconds half_second(500);
    std::cout << "Half second is " << half_second.count() << " milliseconds." << std::endl;

    // 创建一个表示2.5分钟的时间段
    auto time = 2.5min;
    std::cout << "2.5 minutes is " << time.count() << " minutes." << std::endl;
}

void duration_wait()
{
    using namespace std::chrono;

    std::future<int> f = std::async([]() {
        std::this_thread::sleep_for(seconds(1));
        return 42;
    });

    // 等待future任务完成，超时35毫秒
    if (f.wait_for(milliseconds(35)) == std::future_status::ready) {
        std::cout << "Task finished with result: " << f.get() << std::endl;
    } else {
        std::cout << "Timeout reached before task finished." << std::endl;
    }
}

/*
std::chrono::time_point 是表示时钟的一个时间点，通常用于记录时间戳。
时间点通常是一个时钟的一个固定时刻，表示为从某个时间点（通常是 Unix 时间戳或时钟的 "epoch"）到当前时间的时间间隔。
time_point 是基于一个特定的时钟类型来定义的，
它与时间段 (std::chrono::duration) 相对应，可以通过 time_since_epoch() 函数获取到一个时间点距离 epoch 的间隔。
*/

void timepoint()        //获取当前时间点并与 Unix 时间戳进行比较
{
    using namespace std::chrono;

    // 获取系统时钟的当前时间点
    system_clock::time_point now = system_clock::now();

    // 获取时间点与 epoch 的时间差
    auto duration_since_epoch = now.time_since_epoch();
    auto seconds = duration_cast<std::chrono::seconds>(duration_since_epoch).count();

    std::cout << "Current time since epoch (seconds): " << seconds << std::endl;
}

void add_sub()      //进行加法和减法操作
{
    using namespace std::chrono;

    // 获取当前时间点
    high_resolution_clock::time_point now = high_resolution_clock::now();

    // 计算500毫秒后的时间点
    high_resolution_clock::time_point later = now + milliseconds(500);

    std::cout << "Current time point: " << duration_cast<milliseconds>(now.time_since_epoch()).count() << "ms\n";
    std::cout << "Time point after 500ms: " << duration_cast<milliseconds>(later.time_since_epoch()).count() << "ms\n";

}

void code_run_time()
{
    using namespace std::chrono;

    // 获取当前时间点
    high_resolution_clock::time_point start = high_resolution_clock::now();

    // 模拟一些耗时的操作
    std::this_thread::sleep_for(milliseconds(300));

    // 获取结束时间点
    high_resolution_clock::time_point end = high_resolution_clock::now();

    // 计算两时间点之间的时间差
    duration<double> elapsed = duration_cast<duration<double>>(end - start);

    std::cout << "Time taken: " << elapsed.count() << " seconds\n";
}

/// @brief //////////////////
/// @return 

std::condition_variable cv;
std::mutex m;
bool done = false;

void time_point_wait()  //使用时间点进行条件变量等待
{
    // 设置超时的时间点：当前时间 + 500毫秒
    auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);

    std::unique_lock<std::mutex> lk(m);
    while (!done) {
        // 等待条件变量直到超时或条件满足  如果在500毫秒内条件没有满足，程序会超时并退出
        /*
        当使用 std::condition_variable::wait_for() 或 std::condition_variable::wait_until() 时，
        如果在给定的时间段内条件没有满足，返回 std::cv_status::timeout
        */
        if (cv.wait_until(lk, timeout) == std::cv_status::timeout) {    //通过 cv.wait_until() 设置了一个超时的时间点
            std::cout << "Timeout reached!" << std::endl;
            break;
        }
    }
}

int main() {
    int num = 0;
    while(1){
        std::cout << "========================================\n";
        std::cout << "input num to select mod:(-1 exit!)\n";
        std::cin >> num;
        switch (num)
        {
        case 1:
            system_clock();
            break;
        case 2:
            steady_clock();
            break;
        case 3:
            high_resolution_clock();
            break;
        case 4:
            duration_test();
            break;
        case 5:
            duration_wait();
            break;
        case 6:
            timepoint();
            break;
        case 7:
            add_sub();
            break;
        case 8:
            code_run_time();
            break;
        case 9:
            time_point_wait();
            break;
        default:
            break;
        }
        if(num == -1) break;
    }
    return 0;
}
