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

constexpr bool isPrime(int n)
{
    // Corner cases
    if (n <= 1)  return false;
    if (n <= 3)  return true;
   
    // This is checked so that we can skip 
    // middle five numbers in below loop
    if (n%2 == 0 || n%3 == 0) return false;
   
    for (int i=5; i*i<=n; i=i+6)
        if (n%i == 0 || n%(i+2) == 0)
           return false;
   
    return true;
}
 
constexpr int nextPrime(int N)
{
    // Base case
    if (N <= 1)
        return 2;
 
    int prime = N;
    bool found = false;
 
    // Loop continuously until isPrime returns
    // true for a number greater than n
    while (!found) {
        prime++;
 
        if (isPrime(prime))
            found = true;
    }
 
    return prime;
}
 
 

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
    using lake = std::pair<const cstr<3>, int>;


    //std::unordered_map<key, value, std::hash<key>, std::equal_to<key>, lake_alloc<item> > m;
    //std::map<key, value, std::less<key>, lake_alloc<item> > m;
		HashMap<key, value> m;

		for (int i=0;i<7;i++)
			m[i]="100";

			//m.put(i,"100");


    std::cout << "next prime:" << nextPrime(1000000) << std::endl;

		fixed_vector<std::string> s;
		s.push_back("one");
		s.push_back("two");
		std::cout  << "s:" <<  s.size() << " v:" << s[0] << std::endl;
		
    return 0;
}

