#include <iostream>
#include <vector>
#include <list>
#include <map>
#include "allocator.h"

template <typename T>
using allocator = my_allocator::stack::allocator<T>;

int main (int, char **) {

    std::cout << "hello world\n";
    std::vector<int, allocator<int>> v = { 1, 2 ,3 };
    v.push_back(100);
    v.push_back(300);
    for (int value : v)
        std::cout << value << ' ';

    //std::map<const int, int, std::less<int>, allocator<std::pair<int, int>>> m;
    //m[40] = 40;
    //m[20] = 20;
    //m[10] = 10;
    //
    //for (auto& [key, value] : m)
    //    std::cout << key << ' ' << value << ' ';

    std::cin.get();
    return 0;
}
