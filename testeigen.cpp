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


const size_t nrows = 2;
const size_t ncols = 5;

typedef Eigen::Array<short, nrows, ncols> array_type;
typedef Eigen::Map<array_type> mapped_type;

typedef Eigen::Array<short, nrows,1> carray_type;
typedef Eigen::Map<carray_type> cmapped_type;

cmapped_type get_col(mapped_type& arr, int col)
{
    cmapped_type ret(arr.col(col).data());
    return ret;
}

int main()
{

    short buf[nrows*ncols] = { 0,1,2,3,4,5,6,7,8,9 };

    mapped_type rbuf(buf);

    cerr << rbuf << endl;
    buf[4] = 42;
    cerr << rbuf << endl;

    array_type rarr2 = rbuf*rbuf;

    cerr << "rar2:\n" << rarr2 << endl;

    cmapped_type carr = get_col(rbuf, 1);
    cerr << "rbuf:\n" << rbuf << endl;
    cerr << "carr:\n" << carr << endl;
    carr(0) = 22;
    rbuf(1,1) = 33;
    cerr << "rbuf:\n" << rbuf << endl;
    cerr << "carr:\n" << carr << endl;
}
