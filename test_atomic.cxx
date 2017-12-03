// c++-11
#include <thread>
#include <atomic>
#include <algorithm>
#include <vector>
#include <iostream>

std::atomic<int> count(0);
const int ntries = 10000;

void do_inc(int inc) {
    for (int ind=0; ind<ntries; ++ind) {
        {
            int tmp = count;
            count += inc;
        }
    }
}

int main()
{
    std::thread t1(do_inc, +1);
    std::thread t2(do_inc, -1);
    t1.join();
    t2.join();
    std::cout << count << std::endl;
}
