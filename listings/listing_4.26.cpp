#include <experimental/barrier>
#include <thread>
#include <vector>

class joining_thread{
public:
    joining_thread();
    
    template<typename Func>
    joining_thread(Func&&){}
    
    joining_thread(joining_thread&&);
    joining_thread& operator=(joining_thread&&);
};

class data_block {};
class data_chunk {};
class result_chunk {};
class result_block {
public:
    void set_chunk(unsigned index,unsigned total_threads,result_chunk data);
};

class data_source {
  public:
    bool done();
    data_block get_next_data_block();
};

class data_sink {
  public:
    void write_data(result_block data);
};

result_chunk process(data_chunk);
std::vector<data_chunk>
divide_into_chunks(data_block data, unsigned num_threads);

void process_data(data_source &source, data_sink &sink) {
    unsigned const concurrency = std::thread::hardware_concurrency();
    unsigned const num_threads = (concurrency > 0) ? concurrency : 2;

    std::experimental::barrier sync(num_threads);
    std::vector<joining_thread> threads(num_threads);

    std::vector<data_chunk> chunks;
    result_block result;

    for (unsigned i = 0; i < num_threads; ++i) {
        threads[i] = joining_thread([&, i] {
            while (!source.done()) {
                if (!i) {
                    data_block current_block = source.get_next_data_block();
                    chunks = divide_into_chunks(current_block, num_threads);
                }
                sync.arrive_and_wait();
                result.set_chunk(i, num_threads, process(chunks[i]));
                sync.arrive_and_wait();
                if (!i) {
                    sink.write_data(std::move(result));
                }
            }
        });
    }
}
