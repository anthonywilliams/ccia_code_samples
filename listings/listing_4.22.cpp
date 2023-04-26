#include <future>
std::future<FinalResult> process_data(std::vector<MyData>& vec)
{
    size_t const chunk_size=whatever;
    size_t remaining_size, this_chunk_size;
    std::vector<std::future<ChunkResult>> results;
    for(auto it=vec.begin(),end=vec.end();it!=end;){
        remaining_size=end-it;
        this_chunk_size=std::min(remaining_size,chunk_size);
        results.push_back(
            std::async(process_chunk,it,it+this_chunk_size));
        it+=this_chunk_size;
    }
    return std::async([all_results=std::move(results)] () mutable {
        std::vector<ChunkResult> v;
        v.reserve(all_results.size());
        for(auto& f: all_results)
        {
            v.push_back(f.get());
        }
        return gather_results(v);
    });
}
