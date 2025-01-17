#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x_then_y()
{
    x.store(true, std::memory_order_relaxed);           // 1
    std::atomic_thread_fence(std::memory_order_release);    // add relaxed fense, 使得y使用了memory_order_release一样
    y.store(true, std::memory_order_relaxed);
}
                                                                //两个fence形成同步, 使得1始终在6前完成
void read_y_then_x()
{
    while(!y.load(std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);   // add acquire fence, 如同y使用了memory_order_acquire 一样
    if(x.load(std::memory_order_relaxed))                  // 6
        ++z;
}

int main()
{
    x,y = false, false;
    z = 0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    assert(z.load() != 0);
    return 0;
}