#include <memory>
#include <atomic>
template<typename T>
class lock_free_queue
{
private:    struct node;
    struct node_counter
    {
        unsigned internal_count:30;
        unsigned external_counters:2;
    };

    struct counted_node_ptr
    {
        int external_count;
        node* ptr;
    };
    struct node
    {
        std::shared_ptr<T> data;
        std::atomic<int> internal_count;
        counted_node_ptr next;
        node(T const& data_):
            data(std::make_shared<T>(data_)),
            internal_count(0)
        {}
        void release_ref()
        {
            node_counter old_counter=
                count.load(std::memory_order_relaxed);
            node_counter new_counter;
            do
            {
                new_counter=old_counter;
                --new_counter.internal_count;
            }
            while(!count.compare_exchange_strong(
                      old_counter,new_counter,
                      std::memory_order_acquire,std::memory_order_relaxed));
            if(!new_counter.internal_count &&
               !new_counter.external_counters)
            {
                delete this;
            }
        }
    };
    std::atomic<counted_node_ptr> head;
    std::atomic<counted_node_ptr> tail;


};
