// c++-11
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <iostream>

int count = 0;
std::mutex count_mutex;

std::vector<int> values;
const int ntries = 10000;

void do_inc(int inc) {
    for (int ind=0; ind<ntries; ++ind) {
        {
            std::lock_guard<std::mutex> guard(count_mutex);
            values.push_back(count);
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
    auto mm = std::minmax_element(values.begin(), values.end());
    std::cout << *mm.first << " " << count << " " << *mm.second << std::endl;
}
