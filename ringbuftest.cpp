/*
  2^32 * 2560 * 2B = 22. TB = 36. minute
  2^28 * 2560 * 2B = 1.4 TB = 2.2 minute
  2^24 * 2560 * 2B = 86. GB = 8.4 second
  2^20 * 2560 * 2B = 5.4 GB = 0.5 second
 */

#include <boost/circular_buffer.hpp>
#include <iostream>

template<typename sample_type, int ncollection=480, int ninduction=800>
struct Tick {
    sample_type samples[2*(ncollection+ninduction)];
    Tick() {};
};


int main()
{
    typedef Tick<short> tick_type;
    typedef std::shared_ptr<tick_type> tick_ptr;

    const int bufsize = 10;
    typedef boost::circular_buffer<tick_ptr> ring_type;
    
    ring_type ring(bufsize);
    std::cerr << ring.size() << std::endl;

    ring.push_back(std::make_shared<tick_type>());
    ring.push_back(std::make_shared<tick_type>());

    std::cerr << "size=" << ring.size()
              << " reserve=" << ring.reserve()
              << " capacity=" << ring.capacity()
              << " max_size=" << ring.max_size()
              << " empty=" << ring.empty()
              << " full=" << ring.full()
              << std::endl;
    auto one = ring.front();
    one->samples[0] = 0;

    return 0;
}
