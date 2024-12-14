#include"func.h"
#include<vector>
#include<numeric>       //std::accumulate
#include<random>
#include <chrono> // 引入计时所需的头文件

/*
    仿函数： 在一个范围（块）内执行累加
*/
template<typename Iterator,typename T> 
struct accumulate_block 
{ 
  void operator()(Iterator first,Iterator last,T& result) 
  { 
    result=std::accumulate(first,last,result); 
  } 
}; 

/*
并行累加的函数。

1.将输入范围分割为多个块，每个块由一个线程处理。
2.每个线程执行累加操作，然后将结果存储到一个结果向量中。
3.合并每个线程的结果，得到最终的累加结果。
*/
template<typename Iterator,typename T> 
T parallel_accumulate(Iterator first,Iterator last,T init)
{ 
  unsigned long const length=std::distance(first,last); 
 
  if(!length) // 1 输入的范围为空, 得到init
    return init; 
 
  unsigned long const min_per_thread=25; 

  //length+min_per_thread-1: 这里的技巧是使用整数除法来实现“向上取整”  //////////////////////////////////
  unsigned long const max_threads= 
      (length+min_per_thread-1)/min_per_thread; // 2 如果将数据范围分成多个线程，每个线程至少处理 min_per_thread 个元素，那么最多需要多少个线程
 
  unsigned long const hardware_threads= 
      std::thread::hardware_concurrency();      //计算系统中可用的硬件线程数
 
  unsigned long const num_threads=  // 3 确定线程个数
      std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads); 
 
  unsigned long const block_size=length/num_threads; // 4 计算每个线程处理的数据块大小
 
  std::vector<T> results(num_threads); 
  std::vector<std::thread> threads(num_threads-1);  // 5 除了最后一个线程，最后一个线程直接在主线程中执行
 
  Iterator block_start=first; 
  for(unsigned long i=0; i < (num_threads-1); ++i)  //线程处理自动化，批量创建线程
  { 
    Iterator block_end=block_start; 

    //std::advance: 在迭代器上进行相对的增量（或减量）适用于所有类型的迭代器，包括随机访问、双向访问、输入和输出迭代器
    std::advance(block_end,block_size);  // 6 将 block_end 迭代器向前移动 block_size 步，即跳到下一个块的末尾
    threads[i]=std::thread(     // 7 
        accumulate_block<Iterator,T>(), 
        block_start,block_end,std::ref(results[i]));        //传递 results[i] 的引用
    block_start=block_end;  // 8 
  } 
  accumulate_block<Iterator,T>()( 
      block_start,last,results[num_threads-1]); // 9 最后一个线程的累加操作直接在主线程中执行，因为它负责处理剩下的数据（block_start 到 last）
 
  for (auto& entry : threads) 
    entry.join();  // 10    阻塞主线程，直到每个线程完成其任务
 
  return std::accumulate(results.begin(),results.end(),init); // 11 合并结果
}

/*
一个简单的灵活的随机数vector生成器
*/
template<typename T>
std::vector<T> generate_nums(int length, T down, T up)
{
    // 定义随机数生成器和分布
    std::random_device rd;  // 用于获取随机种子     获取硬件或操作系统提供的 真随机数种子
    std::mt19937 gen(rd()); // 用于生成随机数       伪随机数生成器，它基于 梅森旋转算法 其周期非常长（2^19937-1），适合用于生成大规模的随机数

    //生成 均匀分布的实数随机数
    std::uniform_real_distribution<> dis(static_cast<T>(down), static_cast<T>(up)); // 定义一个浮动区间 [0.0f, 100.0f]

    std::vector<T> nums;
    for(int i = 0; i < length; i++){
        nums.push_back(dis(gen));
    }
    return nums;
}

int main()
{
    std::vector<double> nums = generate_nums<double>(1000000, 0, 100.f);
    //ps:如果要显式的指定模板类型
    //parallel_accumulate<std::vector<float>::iterator, float>(numbers.begin(), numbers.end(), 0.0f);
    
    // 串行计算：直接使用 std::accumulate
    auto start_serial = std::chrono::high_resolution_clock::now(); // 计时开始
    double serial_result = std::accumulate(nums.begin(), nums.end(), 0.0);
    auto end_serial = std::chrono::high_resolution_clock::now();   // 计时结束
    std::chrono::duration<double> serial_duration = end_serial - start_serial;
    std::cout << "Serial computation result: " << serial_result << std::endl;
    std::cout << "Time taken for serial computation: " << serial_duration.count() << " seconds" << std::endl;

    // 并行计算：使用 parallel_accumulate
    auto start_parallel = std::chrono::high_resolution_clock::now(); // 计时开始
    double parallel_result = parallel_accumulate(nums.begin(), nums.end(), 0.0);
    auto end_parallel = std::chrono::high_resolution_clock::now();   // 计时结束
    std::chrono::duration<double> parallel_duration = end_parallel - start_parallel;
    std::cout << "Parallel computation result: " << parallel_result << std::endl;
    std::cout << "Time taken for parallel computation: " << parallel_duration.count() << " seconds" << std::endl;
    return 0;
}