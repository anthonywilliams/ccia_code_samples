#include <atomic>
#include <memory>

template<typename T>
class lock_free_stack
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        node* next;
        node(T const& data_):
            data(std::make_shared<T>(data_))
        {}
    };
    std::atomic<node*> head;
public:
    void push(T const& data)
    {
        node* const new_node=new node(data);
        new_node->next=head.load();
        while(!head.compare_exchange_weak(new_node->next,new_node));
    }
    std::shared_ptr<T> pop()
    {
        if (!head_) { return std::shared_ptr<T>(); }
        node* original_head=head.load();
        while(!head.compare_exchange_weak(original_head,original_head->next));
        return original_head->data;
    }
};
