int main()
{
    int x = 42;
    float y = 3.14;
    char z = 'a';
    std::string hello = "Hello, World!";
    if (x > y && z != 'b') {
        y = x + 1;
    } else {
        y = x - 1;
    }
    return 0;
}