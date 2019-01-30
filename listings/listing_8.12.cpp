#include <thread>
#include <atomic>
class barrier
{
    unsigned const count;
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;
public:
    explicit barrier(unsigned count_):
        count(count_),spaces(count),generation(0)
    {}
    void wait()
    {
        unsigned const my_generation=generation;
        if(!--spaces)
        {
            spaces=count;
            ++generation;
        }
        else
        {
            while(generation==my_generation)
                std::this_thread::yield();
        }
    }
};

