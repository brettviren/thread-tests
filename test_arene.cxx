#include "arene.h"
#include <boost/timer/timer.hpp>
#include <thread>


int main()
{
    const int bufsize = 2<<20;
    const int width = 1024; // 2560;

    // 1e8 elements and no ptr memory access 0.45s
    // 1e6 and simple enumerated filling of ptr: 1.5s.
    // 1e6 and memcopy of 1024 in/out: .77 s
    // 1e6 and memcopy of 2560 in/out: 2.5 s
    // 1e6 and memcopy of 4096 in/out: 4.1 s
    const int nelements = 1000000;

    typedef arene::BlockBuffer<short> buffer_type;
    buffer_type queue(width, bufsize);

    boost::timer::auto_cpu_timer timer;
    
    // note: these junk arrays are used inside the write and read
    // threads just to test memory copy speed.  In a real app, they
    // must not be accessed outside their thread.
    const buffer_type::element_type junk[width] = {0};
    buffer_type::element_type junk2[width] = {0};
    buffer_type::element_type* junk2ptr = &junk2[0];

    std::thread write_thread( [&] () {
            for(int i = 0; i<nelements; i++) {
                buffer_type::element_type* ptr=nullptr;
                queue.push(ptr);
                std::memcpy(ptr, junk,
                             width*sizeof(buffer_type::element_type));
            }
        });

    std::thread read_thread( [&] () {
            for(int i = 0; i<nelements; i++) {
                queue.pop(&junk2ptr);
                
            }
        });

    write_thread.join();
    read_thread.join();
    return 0;
}


/* notes;

   hal laptop has 64bit 1600 MHz DDR3 with theoretical transfer rate
   of 12.8 GB/s.

   with 1e6 memcpy's of 1024 shorts in .77 s that's 2.6GB/s.  There's
   also outcopy happening at the same time so 5.3GB/s transfer.  Not
   quite half theoretical.  At width=4096 this is 4GB/s


 */
