#include <iostream>
#include <thread>
#include<assert.h>

void do_something(int& i) {
    std::cout << i << std::endl;
}


typedef struct func{
    int& i;
    func(int& i_) : i(i_) {}

    void operator()() {
        for (unsigned j = 0; j < 10; ++j) {
            do_something(i);
        }
    }
}func;

class BackgroundTask {
public:
int i;
    BackgroundTask(int value) : i(value) {}
    void operator()() {
        do_something(i);
    }
};


