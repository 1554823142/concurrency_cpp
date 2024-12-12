#include <iostream>
#include <thread>
#include <cassert>
#include <string>

#define SKIP_EDIT_DOCUMENT  // 注释掉这一行，表示编译 `edit_document` 部分代码

// 假设这部分是你希望在某些情况下跳过的代码
void do_something(int& i) {
    std::cout << i << std::endl;
}

// 定义 BackgroundTask 类
class BackgroundTask {
public:
    int i;
    BackgroundTask(int value) : i(value) {}
    void operator()() {
        do_something(i);
    }
};

// demo1 示例函数
void demo1()
{
    BackgroundTask do_background_work(4);

    std::thread t(do_background_work);
    t.detach();
    assert(!t.joinable());      //detach后就无法join了，也不能对没有执行线程的对象执行detach
    //t.join();
}

/*
处理文档实例 ： 每处理一个文档开一个线程没有必要，对于相对独立的文档操作，
可以同时进行多个操作，让处理窗口运行在**分离线程**上
*/
#ifndef SKIP_EDIT_DOCUMENT
void edit_document(std::string const& filename)
{
    open_document_and_display_gui(filename);

    while (!done_editing())
    {
        user_command cmd = get_user_input();

        // 使用条件编译跳过某些部分的代码
        
        if (cmd.type == open_new_document)
        {
            std::string const new_name = get_filename_from_user();
            std::thread t(edit_document, new_name);  // 1
            t.detach();  // 2
        }
        
        else
        {
            process_user_input(cmd);
        }
    }
}
#endif

int main()
{
    //demo1();
    return 0;
}
