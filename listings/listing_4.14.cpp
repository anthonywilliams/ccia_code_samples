#include <future>

template <typename Func, typename ...Args>
std::future<std::invoke_result_t<Func&&, Args&&...>> spawn_task(Func &&f, Args &&...rest) {
    typedef std::invoke_result_t<Func&&, Args&&...> result_type;
    
    std::packaged_task<result_type(Args&&...)> task(std::move(f));
    
    std::future<result_type> result(task.get_future());
    
    std::thread worker(std::move(task), std::move(rest)...);
    worker.detach();
    
    return result;
}
