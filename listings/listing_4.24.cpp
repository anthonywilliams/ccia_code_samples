#include <atomic>
#include <experimental/future>
#include <memory>
#include <vector>
struct MyData {};
struct FinalResult {};

bool matches_find_criteria(MyData const &);
FinalResult process_found_value(MyData const &);

std::experimental::future<FinalResult>
find_and_process_value(std::vector<MyData> &data) {
    unsigned const concurrency = std::thread::hardware_concurrency();
    unsigned const num_tasks = (concurrency > 0) ? concurrency : 2;
    std::vector<std::experimental::future<MyData *>> results;
    auto const chunk_size = (data.size() + num_tasks - 1) / num_tasks;
    auto chunk_begin = data.begin();
    std::shared_ptr<std::atomic<bool>> done_flag =
        std::make_shared<std::atomic<bool>>(false);
    for (unsigned i = 0; i < num_tasks; ++i) {
        auto chunk_end =
            (i < (num_tasks - 1)) ? chunk_begin + chunk_size : data.end();
        results.push_back(std::experimental::async([=] {
            for (auto entry = chunk_begin; !*done_flag && (entry != chunk_end);
                 ++entry) {
                if (matches_find_criteria(*entry)) {
                    *done_flag = true;
                    return &*entry;
                }
            }
            return (MyData *)nullptr;
        }));
        chunk_begin = chunk_end;
    }
    std::shared_ptr<std::experimental::promise<FinalResult>> final_result =
        std::make_shared<std::experimental::promise<FinalResult>>();
    struct DoneCheck {
        std::shared_ptr<std::experimental::promise<FinalResult>> final_result;

        DoneCheck(
            std::shared_ptr<std::experimental::promise<FinalResult>>
                final_result_)
            : final_result(std::move(final_result_)) {}

        void operator()(
            std::experimental::future<std::experimental::when_any_result<
                std::vector<std::experimental::future<MyData *>>>>
                results_param) {
            auto results = results_param.get();
            MyData *const ready_result = results.futures[results.index].get();
            if (ready_result)
                final_result->set_value(process_found_value(*ready_result));
            else {
                results.futures.erase(results.futures.begin() + results.index);
                if (!results.futures.empty()) {
                    std::experimental::when_any(
                        results.futures.begin(), results.futures.end())
                        .then(std::move(*this));
                } else {
                    final_result->set_exception(
                        std::make_exception_ptr(                    
                            std::runtime_error(“Not found”)));
                }
            }
        }
    };

    std::experimental::when_any(results.begin(), results.end())
        .then(DoneCheck(final_result));
    return final_result->get_future();
}
