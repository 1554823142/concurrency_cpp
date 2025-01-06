#include <atomic>
#include <thread>
#include <assert.h>
#include <iostream>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
    x.store(true, std::memory_order_seq_cst);  // 1
}

void write_y() {
    y.store(true, std::memory_order_seq_cst);  // 2
}

void read_x_then_y() {
    while (!x.load(std::memory_order_seq_cst));
    if (y.load(std::memory_order_seq_cst))  // 3 load x早于 load y
        ++z;
}

void read_y_then_x() {
    while (!y.load(std::memory_order_seq_cst));
    if (x.load(std::memory_order_seq_cst))  // 4 若 3 处返回为false, 则此处的必返回为true(因为while可以保证在某一时刻y为true)
        ++z;
}

int main() {
    x = false;
    y = false;
    z = 0;

    std::thread a(write_x);
    std::thread b(write_y);
    std::thread c(read_x_then_y);
    std::thread d(read_y_then_x);

    a.join();
    b.join();
    c.join();
    d.join();

    assert(z.load() != 0);  // 5  永远不会触发, 因为总有x,y发生
    std::cout << "z = " << z << std::endl;
    return 0;
}
