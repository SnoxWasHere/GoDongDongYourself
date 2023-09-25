// Online C++ compiler to run C++ program online
#include <iostream>
using namespace std;

inline int round(int num)
{
    return (num % 16 != 0) ? ((num + 16) / 16) : (num / 16);
}

int main() {

    for (size_t i = 0; i <= 200; i += 5)
    {
        int rounded = round(i);
        cout << i << " - " << rounded*16 << endl;
        string output = (rounded*16 - i >= 0 && rounded*16 - i < 16) ? "passed" : "failed";
        cout << output << endl;
    }
    

    return 0;
}