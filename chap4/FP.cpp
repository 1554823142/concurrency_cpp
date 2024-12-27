#include <list>
#include <iostream>
#include <algorithm>
#include <future>

//串行版quick_sort
template<typename T>
std::list<T> sequetial_quick_sort(std::list<T> input)
{
    if(input.empty()) return input;
    std::list<T> result;
    //void splice (const_iterator pos, list& x, const_iterator i); 将一个列表中的一个元素插入到另一个列表的某个位置
    result.splice(result.begin(), input, input.begin());    //将输入的首个元素(中间值)放入结果列表中
    T const& pivot = *result.begin();                       // 它不会复制元素，而是直接连接或切断元素的链表结构

    //用std::partition将序列中的值分成小于“中间”值的组和大于“中间”值的组
    /*
    partition: 通过调用一个谓词函数（即一个返回 true 或 false 的函数），将满足该条件的元素与不满足该条件的元素分开。
    被划分为“真”部分的元素会被移动到容器的前面，而“假”部分的元素会被移动到容器的后面。
    */
    auto divide_point = std::partition(input.begin(), input.end(), 
                        [&](T const& t){return t < pivot;});

    std::list<T> lower_part;
    //input列表小于divided_point的值移动到新列表lower_part, 其余部分保留在input中
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower(sequetial_quick_sort(std::move(lower_part)));
    auto new_higher(sequetial_quick_sort(std::move(input)));

    //拼接result结果 new_higher指向的值放在“中间”值的后面，new_lower指向的值放在“中间”值的前面
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }

    std::list<T> result;
    result.splice(result.begin(), input, input.begin());

    T const& pivot = *result.begin();

    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t) { return t < pivot; });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    std::future<std::list<T> > new_lower(  // 1 使用异步对一部分进行排序, 递归三次就会有2^3个线程
        std::async(&parallel_quick_sort<T>, std::move(lower_part))
    );

    auto new_higher = parallel_quick_sort(std::move(input));  // 2

    result.splice(result.end(), new_higher);  // 3
    result.splice(result.begin(), new_lower.get());  // 4

    return result;
}


int main()
{
    std::list<int> input{5, 7, 3, 4, 1, 9, 2, 8, 10, 6};
    auto ans = sequetial_quick_sort(input);
    std::cout << "sequetial_quick_sort begin!\n";
    for(auto i:ans)
        std::cout << i << " ";
    std::cout << std::endl;

    auto ans2 = parallel_quick_sort(input);
    std::cout << "parallel_quick_sort begin!\n";
    for(auto i:ans)
        std::cout << i << " ";
    std::cout << std::endl;
    return 0;
}