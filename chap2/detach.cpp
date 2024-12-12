#include "func.h"

int main()
{
    BackgroundTask do_background_work(4);
    std::thread t(do_background_work);
    t.detach();
    assert(!t.joinable());
}