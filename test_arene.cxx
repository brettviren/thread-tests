#include "arene.h"
#include <boost/timer/timer.hpp>
#include <thread>
#include <chrono>
#include <random>               // only if seed is given
#include <iostream>


int main(int argc, char* argv[])
{
    int nelements = 100000000;
    int nbits = 18;
    int width = 4096;
    std::size_t latency = 0;

    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " nelements [nbits] [width] [latency] [snooze] [seed]\n";
        std::cout << "any zero values will use defaults: "
                  << nelements << " " << nbits << " " << width << " " << latency << " 0 0\n";
        std::cout << "nonzero seed will fill input array with random values\n";
        return 1;
    }

    // 1e8 elements and no ptr memory access 0.45s
    // 1e6 and simple enumerated filling of ptr: 1.5s.
    // 1e6 and memcopy of 1024 in/out: .77 s
    // 1e6 and memcopy of 2560 in/out: 2.5 s
    // 1e6 and memcopy of 4096 in/out: 4.1 s
    if (argc > 1) {
        int tmp = atoi(argv[1]);
        if (tmp > 0) {
            nelements = tmp;
        }
    }

    if (argc > 2) {
        int tmp = atoi(argv[2]);
        if (tmp > 0) {
            nbits = tmp;
        }
    }
    int bufsize = 2<<nbits;
    if (argc > 3) {
        int tmp = atoi(argv[3]);
        if (tmp > 0 ) {
            width = tmp;
        }
    }
    if (argc > 4) {
        int tmp = atoi(argv[4]);
        if (tmp > 0) {
            latency = tmp;
        }
    }

    std::chrono::nanoseconds snooze(0);
    if (argc > 5) {
        int tmp = atoi(argv[5]);
        if (tmp > 0) {
            snooze = std::chrono::nanoseconds(tmp);
        }
    }

    int seed=0;
    if (argc > 6) {
        int tmp = atoi(argv[6]);
        if (tmp > 0 ) {
            seed = tmp;
        }
    }

    typedef arene::BlockBuffer<short> buffer_type;
    buffer_type queue(width, bufsize);

    std::cout
        << "sample: " << sizeof(buffer_type::element_type) << "\n"
        << "bits: " << nbits << "\n"
        << "depth: " << bufsize << "\n"
        << "width: " << width << "\n"
        << "nticks: " << nelements << "\n"
        << "latency: " << latency << "\n"
        << "snooze: " << snooze.count() << "\n"
        << "seed: " << seed << "\n";

    
    // note: these junk arrays are used inside the write and read
    // threads just to test memory copy speed.  In a real app, they
    // must not be accessed outside their thread.
    buffer_type::element_type junk[width] = {0};
    buffer_type::element_type junk2[width] = {0};
    buffer_type::element_type* junk2ptr = &junk2[0];
    if (seed) {
        std::default_random_engine generator;
        std::normal_distribution<float> gauss(2048, 100);
        auto rng = std::bind (gauss, generator);
        for (int ind=0; ind<width; ++ind) {
            float val = rng();
            if (val < 0) {
                val=0;
            }
            junk[ind] = (short)val;
        }
    }        

    // ready... set... go!
    boost::timer::auto_cpu_timer timer(std::cout,3);

    std::thread write_thread( [&] () {
            int expo = 2;
            for(int i = 0; i<nelements; i++) {
                if (i&expo) {
                    expo *= 2;
                    //std::cerr << "w: " << i << std::endl;
                }
                buffer_type::element_type* ptr=nullptr;
                queue.push(ptr);
                std::memcpy(ptr, junk,
                             width*sizeof(buffer_type::element_type));
            }
        });

    std::thread read_thread( [&] () {
            for(int i = 0; i<nelements; i++) {
                
                if (latency > 0) {
                    while (true) {
                        const std::size_t head = queue.head();
                        const std::size_t tail = queue.tail();
                        if (head-tail > latency or head == nelements) {
                            break;
                        }
                        //std::cerr << "sleeping with: " << head << " " << tail << std::endl;
                        std::this_thread::sleep_for(snooze);
                    };
                }                    
                queue.pop(&junk2ptr);                
            }
        });

    write_thread.join();
    read_thread.join();

    const boost::timer::cpu_times elapsed(timer.elapsed());

    double seconds = 1.0e-9*elapsed.wall;
    double volume = nelements * sizeof (buffer_type::element_type) * width;
    double data_throughput = volume/seconds;
    double transaction_tp = nelements / seconds;

    std::cout << "time: " << seconds << " s\n"
              << "volume: " << volume*1.0e-9 << " GB\n"
              << "data: " << data_throughput*1.0e-9 << " GB/s\n"
              << "ticks: " << transaction_tp*1.0e-6 << " MT/s\n"
              << "times: ";     // filled in by timer's dtor
    
    return 0;
}


/* notes;

   hal laptop has 64bit 1600 MHz DDR3 with theoretical transfer rate
   of 12.8 GB/s.

   with 1e6 memcpy's of 1024 shorts in .77 s that's 2.6GB/s.  There's
   also outcopy happening at the same time so 5.3GB/s transfer.  Not
   quite half theoretical.  At width=4096 this is 4GB/s


 */
