#include <iostream>

class David
{
public:
    int cow;
    David(int c) {cow = c;}
    ~David()
    {
        std::cout << "hi!";
    }
};

int main()
{
    David dave(5);
}
