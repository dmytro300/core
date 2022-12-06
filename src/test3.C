#include "alloc.H"
#include "meta.H"
#include <unordered_map>
#include <iostream>

#include <iomanip>
#include <utility>
#include <vector>
#include <string>

#include <array>
#include <unordered_map>
#include <vector>

template <typename T>
class krystal_alloc {

public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    pointer allocate(size_type n) {
			std::cout  << "alloc sz:" << n << std::endl;
      return reinterpret_cast<pointer>(malloc((sizeof(T) * n)));
    }

    void deallocate(pointer p, size_type n) {
			std::cout  << "dealloc p:" <<  p << " sz:" << n << std::endl;
    }
};


/*
template <class U>
    krystal_alloc(const krystal_alloc<U, Alloc>& u)
        : allocator_(u.allocator_) {}
*/

// -- standard usage
template <typename T>
using lake_alloc = krystal_alloc<T>;

int main()
{
    using key = int;
    using value = std::string;
    using item = std::pair<const key, value>;

    //std::unordered_map<key, value, std::hash<key>, std::equal_to<key>, lake_alloc<item> > m;
    //std::map<key, value, std::less<key>, lake_alloc<item> > m;
		HashMap<key, value> m;

		for (int i=0;i<70;i++)
			m.put(i,"100");

		m.remove(3);
		m.remove(5);
		m.remove(6);

		//std::cout  << "m:" <<  m[1] << std::endl;

    return 0;
}

