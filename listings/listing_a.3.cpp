class CX
{
private:
    int a;
    int b;
public:
    CX() = default;
    CX(int a_, int b_):
        a(a_),b(b_)
    {}
    int get_a() const
    {
        return a;
    }
    int get_b() const
    {
        return b;
    }
    int foo() const
    {
        return a+b;
    }
};
