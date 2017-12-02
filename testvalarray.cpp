#include <valarray>
#include <iostream>

template<typename ARR>
void dump(const ARR& arr, size_t len)
{
    for (size_t ind=0; ind<len; ++ind) {
	std::cerr << ind << ":" << arr[ind] << " ";
    }
    std::cerr << std::endl;
}

int main()
{
    int buf[10] = { 0,1,2,3,4,5,6,7,8,9 };
    std::valarray<int> v1(buf, 10);
    std::valarray<int> v2;
    v2 = std::valarray<int>(buf, 10);


    v1[1] = 41;
    v2[2] = 42;
			  

    std::valarray<int> s0 = v1[std::slice(0*5, 5, 1)];
    std::valarray<int> s1 = v1[std::slice(1*5, 5, 1)];    
    s0[3] = 30;
    s1[3] = 31;

    dump(buf, 10);
    dump(v1, 10);
    dump(v2, 10);
}
