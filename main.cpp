#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <ranges>
#include <algorithm>
#include <numeric>
#include "allocator.h"

using allocator = my_allocator::heap::allocator<std::pair<const int, int>>;

int main (int, char **) 
{
    allocator alloc(10);
    std::map<int, int, std::less<int>, allocator> m1(alloc);
    m1[0] = 1;
    for (int i = 1; i < 9; ++i)
    {
        m1[i] = i * m1[i - 1];
    }

    for (auto& [key, value] : m1)
        std::cout << key << ' ' << value << '\n';

    std::cin.get();
    return 0;
}
