#include <iostream>
#include <thread>

void thread1(std::thread& t2) {
    std::cout << "Thread 1 is waiting for Thread 2 to finish...\n";
    t2.join();  // Thread 1 waits for Thread 2 to finish
    std::cout << "Thread 1 finished.\n";
}

void thread2(std::thread& t1) {
    std::cout << "Thread 2 is waiting for Thread 1 to finish...\n";
    t1.join();  // Thread 2 waits for Thread 1 to finish
    std::cout << "Thread 2 finished.\n";
}

int main() {
    std::thread t1; // Declare threads
    std::thread t2;

    // Start thread1 and thread2, passing references to each other for join
    t1 = std::thread(thread1, std::ref(t2));  
    t2 = std::thread(thread2, std::ref(t1));

    // Wait for both threads to finish (but they never will)
    t1.join();
    t2.join();

    return 0;
}
