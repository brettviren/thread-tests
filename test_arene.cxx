#include "arene.h"
#include <boost/timer/timer.hpp>
#include <thread>
#include <chrono>
#include <random>               // only if seed is given
#include <iostream>

#include "json.hpp"
using json = nlohmann::json;

int main(int argc, char* argv[])
{
    int nelements = 100000000;
    int nbits = 18;
    int width = 4096;
    std::size_t latency = 0;

    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " nelements [nbits] [width] [latency] [snooze] [seed]\n";
        std::cerr << "any zero values will use defaults: "
                  << nelements << " " << nbits << " " << width << " " << latency << " 0 0\n";
        std::cerr << "nonzero seed will fill input array with random values\n";
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

    int snooze_ns = 0;
    if (argc > 5) {
        int tmp = atoi(argv[5]);
        if (tmp > 0) {
            snooze_ns = tmp;
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
    boost::timer::auto_cpu_timer timer(std::cerr,3);

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
                    std::chrono::nanoseconds snooze(snooze_ns);
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

    {

        json jbuffer;
        jbuffer["element_size"] = (int)sizeof(buffer_type::element_type);
        jbuffer["log2depth"] = nbits;
        jbuffer["depth"] = bufsize;
        jbuffer["width"] = width;
        
        json jparam;
        jparam["nticks"] = nelements;
        jparam["latency"] = latency;
        jparam["snooze_ns"] = snooze_ns;
        jparam["seed"] = seed;

        json jres;
        jres["wall"] = 1.0e-9*elapsed.wall;
        jres["system"] = 1.0e-9*elapsed.system;
        jres["user"] = 1.0e-9*elapsed.user;
        jres["volume"] = volume;
        jres["data_tp"] = data_throughput;
        jres["tick_tp"] = transaction_tp;

        json jdat;
        jdat["buffer"] = jbuffer;
        jdat["params"] = jparam;
        jdat["results"] = jres;
    
        std::cout << jdat.dump(4) << std::endl;
    }
    return 0;
}


/* notes;

   hal laptop has 64bit 1600 MHz DDR3 with theoretical transfer rate
   of 12.8 GB/s.

   with 1e6 memcpy's of 1024 shorts in .77 s that's 2.6GB/s.  There's
   also outcopy happening at the same time so 5.3GB/s transfer.  Not
   quite half theoretical.  At width=4096 this is 4GB/s


 */
