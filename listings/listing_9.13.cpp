#include <vector>
#include <mutex>
#include <thread>
#include <future>

class interrupt_flag
{
public:
    void set();
    bool is_set() const;
};
thread_local interrupt_flag this_thread_interrupt_flag;

class interruptible_thread
{
    std::thread internal_thread;
    interrupt_flag* flag;
public:
    template<typename FunctionType>
    interruptible_thread(FunctionType f)
    {
        std::promise<interrupt_flag*> p;
        internal_thread=std::thread([f,&p]{
                p.set_value(&this_thread_interrupt_flag);
                f();
            });
        flag=p.get_future().get();
    }
    void interrupt()
    {
        if(flag)
        {
            flag->set();
        }
    }
};

std::mutex config_mutex;
std::vector<interruptible_thread> background_threads;

void background_thread(int disk_id)
{
    while(true)
    {
        interruption_point();
        fs_change fsc=get_fs_changes(disk_id);
        if(fsc.has_changes())
        {
            update_index(fsc);
        }
    }
}

void start_background_processing()
{
    background_threads.push_back(
        interruptible_thread(background_thread,disk_1));
    background_threads.push_back(
        interruptible_thread(background_thread,disk_2));
}

int main()
{
    start_background_processing();
    process_gui_until_exit();
    std::unique_lock<std::mutex> lk(config_mutex);
    for(unsigned i=0;i<background_threads.size();++i)
    {
        background_threads[i].interrupt();
    }
    for(unsigned i=0;i<background_threads.size();++i)
    {
        background_threads[i].join();
    }
}
