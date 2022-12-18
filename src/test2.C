#include "alloc.H"
#include "meta.H"
#include <unordered_map>
#include <iostream>

#include <iomanip>
#include <utility>
#include <vector>
#include <string>

using std::cout;
using std::endl;

/**
 * The `Object` structure uses custom allocator,
 * overloading `new`, and `delete` operators.
 */
struct Object {

  // Object data, 16 bytes:
  std::string d;

  // Declare out custom allocator for
  // the `Object` structure:
  static PoolAllocator allocator;

  static void *operator new(size_t) {
    return allocator.allocate();
  }

  static void operator delete(void *ptr, size_t) {
    return allocator.deallocate(ptr);
  }
};

// Instantiate our allocator, using 8 chunks per block:
PoolAllocator Object::allocator{sizeof(Object), 512};

std::unique_ptr<Object> mmm(std::unique_ptr<Object> p)
{
	return p;
}

Object mmm(Object p)
{
	return p;
}

int main(int, char const **) {

  // Allocate 10 pointers to our `Object` instances:
  constexpr int arraySize = 512;

  Object *objects[arraySize];
/*
  for (int i = 0; i < arraySize; ++i) {
    objects[i] = new Object();
  }

  for (int i = 0; i < arraySize; ++i) {
    delete objects[i];
  }

	Object *o = reinterpret_cast<Object*>(reinterpret_cast<char*>(objects)+1);
	delete o;
	delete objects[511];
	delete objects[512];
*/
	std::unique_ptr<Object> p(new Object);
	std::unique_ptr<Object> d(new Object);

	p = mmm (std::move( d ));
	//p = mmm( d );

	
{
		std::unordered_map<cstr<3>, int, cstr_hasher<3>> cmap;
		cmap["123"] = 9;
}
{
		std::map<cstr<3>, int, std::less<>> mmap;
		mmap["123"] = 1;

		cstr<3> k("123");
		std::string_view key{ "123" };
		auto r = mmap.find(key);

		std::cout  << "res:" << r->second << '\n' << std::endl;
}
{
		std::map<std::string, int, std::less<>> tmp;
		tmp["a"] = 0;

		std::string_view key{ "a" };

		if (tmp.find(key) != tmp.end())
				 std::cout << "It works" << std::endl;
}
{
    std::vector<Object> v;
 
		Object o{"hehe"}; 
    // uses the push_back(const T&) overload, which means 
    // we'll incur the cost of copying str
    v.push_back(o);
		std::cout << "After move, str is " << std::quoted(o.d) << '\n';

    // uses the rvalue reference push_back(T&&) overload, 
    // which means no strings will be copied; instead, the contents
    // of str will be moved into the vector.  This is less
    // expensive, but also means str might now be empty.
    v.emplace_back(o);
		mmm(std::move(o));
		std::cout << "After move, str is " << std::quoted(o.d) << '\n';
}
}
