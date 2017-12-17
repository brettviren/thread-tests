#ifndef RINGBUFSPSCMUTEX
#define RINGBUFSPSCMUTEX

#include <boost/circular_buffer.hpp>
#include "bounded_circular_buffer.hpp"

typedef int tick_type;

typedef bounded_buffer<tick_type> RingBufferSPSCMutex;


#endif
