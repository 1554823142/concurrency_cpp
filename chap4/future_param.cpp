#include <string>
#include <future>
#include <iostream>

struct X {
    void foo(int a, std::string const& s) {
        // 这里是 foo 函数的实现
        std::cout << "foo called with: " << a << ", " << s << std::endl;
    }

    std::string bar(std::string const& s) {
        // 这里是 bar 函数的实现
        return "bar called with: " + s;
    }
};

X x;


struct Y {
    double operator()(double val) {
        // 这里是 operator() 的实现
        return val * 2.0;  // 比如将输入的值乘以 2
    }
};

Y y;


X baz(X& obj) {
    // 这里是 baz 函数的实现
    obj.foo(100, "called from baz");
    return obj;
}

class move_only {
public:
    move_only() {
        // 默认构造函数实现
        std::cout << "move_only constructed" << std::endl;
    }

    move_only(move_only&&) {
        // 移动构造函数实现
        std::cout << "move_only moved" << std::endl;
    }

    move_only(move_only const&) = delete;  // 禁止拷贝构造

    move_only& operator=(move_only&&) {
        // 移动赋值运算符实现
        std::cout << "move_only moved via assignment" << std::endl;
        return *this;
    }

    move_only& operator=(move_only const&) = delete;  // 禁止拷贝赋值

    void operator()() {
        // 函数调用运算符实现
        std::cout << "move_only called" << std::endl;
    }
};


int main() {
    // 启动异步任务 f1: 调用 X::foo
    auto f1 = std::async(&X::foo, &x, 42, "hello");     // 调用 p->foo(42, "hello")，p 是指向 x 的指针
    // 启动异步任务 f2: 调用 X::bar
    auto f2 = std::async(&X::bar, x, "goodbye");        // 调用 tmpx.bar("goodbye")，tmpx 是 x 的拷贝副本

    // 启动异步任务 f3: 调用 Y 的 operator()
    auto f3 = std::async(Y(), 3.141);                   // 调用 tmpy(3.141)，tmpy 通过 Y 的移动构造函数得到

    // 启动异步任务 f4: 调用 Y 的 operator()（传递引用）
    auto f4 = std::async(std::ref(y), 2.718);           // 调用 y(2.718)

    // 启动异步任务 f5: 调用 move_only 对象
    /*
    步骤: 1.由于传递的是默认构造函数, 所以会构造一个临时的move_only对象, 
          2. 可调用对象执行, 因为实现了operator()()
          3. 把 move_only() 临时对象传递给 std::async 时，std::async 会将该对象传递给 std::async 的可调用参数。
          这时，move_only 对象会被**移动构造**（而不是拷贝构造），因为传递的是一个**右值**
    
    */
    auto f5 = std::async(move_only());                  // 调用 tmp()，tmp 是通过 std::move(move_only()) 构造得到




    // 启动异步任务 f6: 调用 baz 函数
    auto f6 = std::async(baz, std::ref(x));             // 调用 baz(x)

    // 等待所有异步任务完成并获取结果
    f1.get();
    std::cout << "f2 result: " << f2.get() << std::endl;
    std::cout << "f3 result: " << f3.get() << std::endl;
    std::cout << "f4 result: " << f4.get() << std::endl;
    f5.get();  // f5 是 void 函数调用，不需要返回值
    f6.get();  // f6 返回 X，但我们不需要结果

    return 0;
}