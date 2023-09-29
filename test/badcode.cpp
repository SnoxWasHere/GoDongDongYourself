#include <iostream>
using namespace std;

int main()
{
    unsigned modeNum;
    MODE:
    cout << "Select mode: "; cin >> modeNum; cout << endl;
    switch (modeNum)
    {
    case 1:
        cout << 1 << endl;
        break;
    case 2:
        cout << 2 << endl;
        break;
    case 3:
        cout << 3 << endl;
        break;
    case 4:
        cout << 4 << endl;
        break;
    default:
        cout << "Invalid mode number." << endl;
        goto MODE;
        break;
    }
}