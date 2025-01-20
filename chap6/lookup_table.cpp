# include <iostream>
# include <list>
# include <mutex>
# include <memory>
#include <thread>
# include <shared_mutex>
#include <algorithm>
#include <vector>

/// 定义接受三个类型的模版类, 其中Hash的作用是使用std::hash计算key的哈希值 (将 Key 类型的对象映射为一个 std::size_t 类型的整数值)
template<typename Key, typename Value, typename Hash=std::hash<Key> >
class threadsafe_lookup_table{
private:
    class bucket_type{          /// 定义哈希表中的 **一个** 桶
    private:
        typedef std::pair<Key, Value> bucket_value;
        typedef std::list<bucket_value> bucket_data;        /// std::list 双向链表, 数据增删高效
        typedef typename bucket_data::iterator bucket_iterator;
        bucket_data data;
        mutable std::shared_mutex mutex;        /// 每个桶分别由独立的共享锁来保护 使用const: 允许即使在 const 成员函数中修改 mutex
                                                /// 即使是只读函数(为常函数 带const后缀), 也可以对桶加锁
        bucket_iterator find_entry_for(Key const& key) const{       /// 检查桶内是否有待查找的key
            return std::find_if(data.begin(), data.end(),
                                [&](bucket_value const& item){
                                    return item.first == key;});
        }
    public:
        Value value_for(Key const& key, Value const& default_value) const{
            std::shared_lock<std::shared_mutex> lock(mutex);    /// shared_lock : 共享方式 只读
            bucket_iterator const found_entry = find_entry_for(key);
            return (found_entry == data.end()) ?
                    default_value : found_entry->second;
        }

        void add_or_update_mapping(Key const& key, Value const& value){
            std::unique_lock<std::shared_mutex> lock(mutex);    /// unique_lock : 独占方式 读/写
            bucket_iterator const found_entry = find_entry_for(key);
            if(found_entry == data.end())
                data.push_back(bucket_value(key, value));
            else
                found_entry->second = value;
        }

        void remove_mapping(Key const& key){
            std::unique_lock<std::shared_mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for();
            if(found_entry != data.end())       /// 找到了待移除的key
                data.erase(found_entry);
        }
    };

    std::vector<std::unique_ptr<bucket_type> > buckets;     /// 放置桶
    Hash hasher;
    bucket_type& get_bucket(Key const& key) const{          /// 由于桶的数量一定, 所以不需要加锁保护
        std::size_t const bucket_index = hasher(key) % buckets.size();
        return *buckets[bucket_index];
    }

public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_type;
    threadsafe_lookup_table(
        unsigned num_buckets=19, Hash const& hasher_=Hash()) :
        buckets(num_buckets), hasher(hasher_){
        for(unsigned i = 0; i < num_buckets; ++i)
            buckets[i].reset(new bucket_type);              /// std::unique_ptr : reset(): 释放当前的对象并替换为新的对象
    }
    /// 删除拷贝构造和拷贝赋值运算符(消除资源共享,数据竞争,资源双重释放的问题)
    threadsafe_lookup_table(threadsafe_lookup_table const& other) = delete;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const& other) = delete;
    Value value_for(Key const& key,
                    Value const& default_value=Value()) const{
        return get_bucket(key).value_for(key, default_value);
    }
    void add_or_update_mapping(Key const& key, Value const& value){
        get_bucket(key).add_or_update_mapping(key, value);
    }
    void remove_mapping(Key const& key){
        get_bucket(key).remove_mapping(key);
    }

};