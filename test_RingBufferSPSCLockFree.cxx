// from: 
// https://nativecoding.wordpress.com/2015/06/17/multithreading-lockless-thread-safe-spsc-ring-buffer-queue/
// https://gist.githubusercontent.com/akhin/092693512c2284d8b313/raw/df6bdcc5685f546257e8cf5770be2cfadd72f7b7/spsc_lockless_sequential.cpp
// https://github.com/akhin/multithreaded_order_matching_engine/blob/master/source/core/concurrency/ring_buffer_spsc_lockfree.hpp


#include "RingBufferSPSCLockFree.h"
#include <boost/timer/timer.hpp>
#include <thread>

int main (int argc, char** argv)
{
    const int bufsize = 100;
    const int nelements = 10000000;
    RingBufferSPSCLockFree<int> queue(bufsize);

    boost::timer::auto_cpu_timer timer;

    
    std::thread write_thread( [&] () {
            for(int i = 0; i<nelements; i++) {
                queue.push(i);
            }
        }  // End of lambda expression
        );
    std::thread read_thread( [&] () {
            for(int i = 0; i<nelements; i++) {
                queue.pop();
            }
        }  // End of lambda expression
        );
    write_thread.join();
    read_thread.join();
     
    return 0;
}
