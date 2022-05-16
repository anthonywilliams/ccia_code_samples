#include <mutex>
#include <stdexcept>
#include <climits>
class hierarchical_mutex
{
    std::mutex internal_mutex;
    unsigned long const instance_level;
    unsigned long previous_level;
    static thread_local unsigned long thread_level;

    void check_lock_order()
    {
        if(thread_level <= instance_level)
        {
            throw std::logic_error("mutex lock order violated");
        }
    }
    void update_levels_on_lock()
    {
        previous_level=thread_level;
        thread_level=instance_level;
    }
    void check_unlock_order()
    {
        if(thread_level != instance_level)
        {
            throw std::logic_error("mutex unlock order violated");
        }
    }
    void update_levels_on_unlock()
    {
        thread_level=previous_level;
    }
public:
    explicit hierarchical_mutex(unsigned long value):
        instance_level(value),
        previous_level(0)
    {}
    void lock()
    {
        check_lock_order();
        internal_mutex.lock();
        update_levels_on_lock();
    }
    void unlock()
    {
        check_unlock_order();
        update_levels_on_unlock();
        internal_mutex.unlock();
    }
    bool try_lock()
    {
        check_lock_order();
        if(!internal_mutex.try_lock())
            return false;
        update_levels_on_lock();
        return true;
    }
};
thread_local unsigned long
    hierarchical_mutex::thread_level(ULONG_MAX);       

int main()
{
    hierarchical_mutex m1(42);
    hierarchical_mutex m2(2000);
    
}
