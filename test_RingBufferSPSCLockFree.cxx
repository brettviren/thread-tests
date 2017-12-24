// from: 
// https://nativecoding.wordpress.com/2015/06/17/multithreading-lockless-thread-safe-spsc-ring-buffer-queue/
// https://gist.githubusercontent.com/akhin/092693512c2284d8b313/raw/df6bdcc5685f546257e8cf5770be2cfadd72f7b7/spsc_lockless_sequential.cpp
// https://github.com/akhin/multithreaded_order_matching_engine/blob/master/source/core/concurrency/ring_buffer_spsc_lockfree.hpp


#include "RingBufferSPSCLockFree.h"
#include "RingBufferSPSCLockFree2.h"

#include <boost/circular_buffer.hpp>
#include "bounded_circular_buffer.hpp"

#include <boost/timer/timer.hpp>
#include <thread>
#include <iostream>

#include "json.hpp"
using json = nlohmann::json;


template<typename buffer_type>
json test_rb(int nelements)
{
    const int nbits = 20;
    const int bufsize = 2<<nbits;
    buffer_type queue(bufsize);

    boost::timer::cpu_timer timer;

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
     
    const boost::timer::cpu_times elapsed(timer.elapsed());

    json jbuffer;
    jbuffer["element_size"] = (int)sizeof(typename buffer_type::element_type);
    jbuffer["log2depth"] = nbits;
    jbuffer["depth"] = bufsize;
    jbuffer["width"] = 1;
    
    json jpar;
    jpar["nelements"] = nelements;

    json jres;
    jres["wall"] = 1.0e-9*elapsed.wall;
    jres["system"] = 1.0e-9*elapsed.system;
    jres["user"] = 1.0e-9*elapsed.user;
    

    json jdat;
    jdat["buffer"] = jbuffer;
    jdat["params"] = jpar;
    jdat["results"] = jres;
    
    return jdat;
}

int main (int argc, char** argv) {
    int nelements = 100000000;
    if (argc > 1) {
        int tmp = atoi(argv[1]);
        if (tmp > 0) {
            nelements = tmp;
        }
    }

    json jdat;
    jdat["mutex"] =
        test_rb< bounded_buffer<int> >(nelements);
    jdat["atomic_modulo"] =
        test_rb< RingBufferSPSCLockFree<int> >(nelements);
    jdat["atomic_bitmask"] =
        test_rb< RingBufferSPSCLockFree2<int> >(nelements);

    std::cout << jdat.dump(4) << std::endl;
    return 0;
}
