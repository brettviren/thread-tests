/*
  2^32 * 2560 * 2B = 22. TB = 36. minute
  2^28 * 2560 * 2B = 1.4 TB = 2.2 minute
  2^24 * 2560 * 2B = 86. GB = 8.4 second
  2^20 * 2560 * 2B = 5.4 GB = 0.5 second
 */

#include <boost/circular_buffer.hpp>
#include <boost/timer/timer.hpp>
#include <iostream>
#include <thread>
#include <vector>

#include "bounded_circular_buffer.hpp"

template<typename sample_type, int ncollection=480, int ninduction=800>
struct Tick {
    sample_type samples[2*(ncollection+ninduction)];
    Tick() {};
};


//typedef Tick<short> tick_type;
//typedef std::shared_ptr<tick_type> tick_ptr;
//typedef boost::circular_buffer<tick_ptr> ring_type;
typedef int tick_type;
//typedef boost::circular_buffer<tick_type> ring_type;
typedef bounded_buffer<tick_type> ring_type;

const int bail = 10000000;

struct Filler {
    ring_type& ring;
    int count;
    Filler(ring_type& ring) : ring(ring), count(0) { }
    void operator()() {
        while (true) {
            // if (ring.full()) {
            //     //std::cerr << "f";
            //     continue;
            // }
            //ring.push_back(std::make_shared<tick_type>());
            //std::cerr << '+' << count << std::endl;
            ring.push_front(count);
            ++count;
            if (count == bail) {
                std::cerr << "Filler bail with\n";
                return;
            }
            continue;
        }
    }
};

struct Drainer {
    ring_type& ring;
    int count;
    Drainer(ring_type& ring) : ring(ring), count(0) { }
    void operator()() {
        while (true) {
            // if (ring.empty()) {
            //     //std::cerr << "e";
            //     continue;
            // }
            //std::cerr << '-' << count << std::endl;
            tick_type tick;
            ring.pop_back(&tick);
            ++count;
            if (count == bail) {
                std::cerr << "Drainer bail with\n";
                return;
            }
            continue;
        }
    }
};

int main()
{
    boost::timer::auto_cpu_timer timer;

    const int bufsize = 100;
    
    ring_type ring(bufsize);

    std::vector<std::thread> threads;

    threads.push_back(std::thread(Filler(ring)));
    threads.push_back(std::thread(Drainer(ring)));

    for (auto& th : threads) {
        th.join();
    }

    // std::cerr << "size=" << ring.size()
    //           << " reserve=" << ring.reserve()
    //           << " capacity=" << ring.capacity()
    //           << " max_size=" << ring.max_size()
    //           << " empty=" << ring.empty()
    //           << " full=" << ring.full()
    //           << std::endl;

    return 0;
}
