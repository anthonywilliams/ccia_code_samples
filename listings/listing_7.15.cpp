void push(T new_value)
{
    std::unique_ptr<T> new_data(new T(new_value));
    counted_node_ptr new_next;
    new_next.ptr=new node;
    new_next.external_count=1;
    for(;;)
    {
        node* const old_tail=tail.load();
        T* old_data=nullptr;
        if(old_tail->data.compare_exchange_strong(
               old_data,new_data.get()))
        {
            old_tail->next=new_next;
            tail.store(new_next.ptr);
            new_data.release();
            break;
        }
    }
}
