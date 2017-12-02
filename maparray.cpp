#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/file_mapping.hpp>

#include <eigen3/Eigen/Dense>

#include <chrono>
#include <random>

#include <iostream>
#include <algorithm>




const char* shmem_name = "MySharedMemory";

template<typename clock>
double timeit(clock& start)
{
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = stop-start;
    return elapsed_seconds.count();
}


template<typename DTYPE>
DTYPE* get_shared_buf(size_t nbytes, const char* name="my_shared_buf")
{
    using namespace boost::interprocess;
    shared_memory_object shm (open_or_create, name, read_write);
    shm.truncate(nbytes);
    mapped_region region(shm, read_write, 0, nbytes);
    return (DTYPE*)region.get_address();
}

template<typename DTYPE>
DTYPE* get_shared_file(size_t nbytes, const char* name="my_shared_buf")
{
    using namespace boost::interprocess;
    file_mapping shm (name, read_write);
    mapped_region region(shm, read_write, 0, nbytes);
    return (DTYPE*)region.get_address();
}

int main(int argc, char* argv[])
{
    using DTYPE = short;

    const size_t nchans = 480;
    const size_t nticks = 10.0e-3 * 2.0e6;
    const size_t nsamples = nchans*nticks;
    const size_t nbytes = nsamples*sizeof(DTYPE);
    
    using namespace boost::interprocess;
    shared_memory_object shm (open_or_create, argv[1], read_write);
    shm.truncate(nbytes);
    mapped_region region(shm, read_write, 0, nbytes);
    DTYPE* buffer = (DTYPE*)region.get_address();

    std::cerr << "Buffer at " << (void*)buffer << " with " << nbytes << " bytes\n";
    
    typedef Eigen::Array<DTYPE, nchans, nticks> CTArray;
    typedef Eigen::Array<DTYPE, nchans, 1> ChanArray;
    typedef Eigen::Array<DTYPE, nticks, 1> TickArray;

    auto rbuf = Eigen::Map<CTArray>(buffer);
    // in Python:
    // arr = numpy.memmap("/dev/shm/maparray-buffer", offset=0, order='C', dtype=numpy.int16, mode='r',shape=(20000, 480))

    auto start = std::chrono::high_resolution_clock::now();

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0, 10);

    for (size_t ich=0; ich<nchans; ++ich) {
	auto wave = rbuf.row(ich);
	for (size_t itick=0; itick<nticks; ++itick) {
	    DTYPE val = itick+10000;
	    if (ich) {		// use channel 0 as a tick index 
		val = distribution(generator);
		val = std::max(std::min(val, (DTYPE)2047), (DTYPE)-2048);
	    }
	    wave(itick) = val;
	}
    }
    std::cerr << "fill " << nchans << " x " << nticks << " elements in "
	      << timeit(start) << std::endl;
    //std::cerr << rbuf << std::endl;

    ChanArray chsum = ChanArray::Zero(), chsum2 = ChanArray::Zero();

    start = std::chrono::high_resolution_clock::now();
    for (size_t itick=0; itick<nticks; ++itick) {
	const auto& bych = rbuf.col(itick);
	chsum += bych;
	chsum2 += bych*bych;
    }
    std::cerr << "read by tick and summary in " << timeit(start) << std::endl;

    auto mean = chsum/nticks;
    auto rms = sqrt(chsum2 - mean*mean);
    //std::cerr << rms << std::endl;

    size_t nonzero = 0;
    for (size_t ind=0; ind != nsamples; ++ind) {
	if (buffer[ind]) {
	    ++nonzero;
	}
    }
    std::cerr << "Non zero buffer elements: " << nonzero << std::endl;

    return 0;
}
