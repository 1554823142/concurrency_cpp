#include <atomic>
#include <thread>
#include <assert.h>
#include <iostream>

std::atomic<bool> x, y;
std::atomic<int> z;


void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));
    if (x.load(std::memory_order_relaxed))  // 4  没有顺序去保证每个操作产生相关值的可见性
        ++z;
}

int main() {
    x = false;
    y = false;
    z = 0;

    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);

    a.join();
    b.join();

    assert(z.load() != 0);  // 5  永远不会触发, 因为总有x,y发生
    std::cout << "z = " << z << std::endl;
    return 0;
}
