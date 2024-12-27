#include <exception>
#include <future>
#include <iostream>
#include <math.h>
#include <unistd.h>

double square_root(double x)
{
    if(x < 0)   throw std::out_of_range("x < 0");
    return sqrt(x);
}

int main()
{
    std::future<double> f = std::async(square_root, -1);
    sleep(3);
    f.get();

    return 0;
}