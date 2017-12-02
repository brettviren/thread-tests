#include <chrono>
#include <valarray>
#include <iostream>

int main()
{
    const size_t nchan = 480;
    const size_t nticks = 3*2e6; // 3 seconds * 2 MHz
    const size_t nsamples = nchan*nticks;

    auto start = std::chrono::high_resolution_clock::now();
    short* a = new short[nsamples];
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = stop-start;
    std::cerr << "allocate in " << elapsed_seconds.count() << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::valarray<short> buf(a, nsamples);
    for (size_t ich=0; ich<nchan; ++ich) {
	std::valarray<short> bytick = buf[std::slice(ich*nticks, nticks, 1)];
	for (size_t ind=0; ind<nticks; ++ind) {
	    bytick[ind] = 0;
	}
    }
    stop = std::chrono::high_resolution_clock::now();
    elapsed_seconds = stop-start;
    std::cerr << "loop with stride in " << elapsed_seconds.count() << std::endl;


    std::valarray<short> chsum((short)0, nchan);
    std::valarray<short> chsum2((short)0, nchan);

    start = std::chrono::high_resolution_clock::now();
    for (size_t ind=0; ind<nticks; ++ind) {
	std::valarray<short> bych(a + (ind*nchan), nchan);
	chsum += bych;
	chsum2 += bych*bych;
    }
    stop = std::chrono::high_resolution_clock::now();
    elapsed_seconds = stop-start;
    std::cerr << "loop by tick in " << elapsed_seconds.count() << std::endl;

    return 0;
}
