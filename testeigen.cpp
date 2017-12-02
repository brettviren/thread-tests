#include <valarray>
#include <iostream>
#include <eigen3/Eigen/Dense>

using namespace std;

template<typename ARR>
void dump(const ARR& arr)
{
    const int nrows = arr.rows();
    const int ncols = arr.cols();
    for (size_t irow = 0; irow != nrows; ++irow) {
	cerr << "row " << irow << ": ";
	for (size_t icol=0; icol != ncols; ++icol) {
	    cerr << " " << arr(irow,icol);
	}
	cerr << "\n";
    }
}

int main()
{


    const size_t nrows = 2;
    const size_t ncols = 5;
    short buf[nrows*ncols] = { 0,1,2,3,4,5,6,7,8,9 };
    typedef Eigen::Array<short, nrows, ncols> RBuf;
    auto rbuf = Eigen::Map<RBuf>(buf);

    cerr << rbuf << endl;
    buf[4] = 42;
    cerr << rbuf << endl;

    auto rbuf2 = rbuf*rbuf;

    cerr << rbuf2 << endl;
}
