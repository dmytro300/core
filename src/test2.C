#include "alloc.H"
#include "meta.H"
#include <unordered_map>
#include <iostream>

using std::cout;
using std::endl;

/**
 * The `Object` structure uses custom allocator,
 * overloading `new`, and `delete` operators.
 */
struct Object {

  // Object data, 16 bytes:
  char data[2];

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
PoolAllocator Object::allocator{sizeof(Object), 8};

int main(int, char const **) {

  // Allocate 10 pointers to our `Object` instances:

  constexpr int arraySize = 10;

  Object *objects[arraySize];

  // Allocate 10 objects. This causes allocating two larger,
  // blocks since we store only 8 chunks per block:

  cout << "About to allocate " << objects[10] << " objects" << endl;

  for (int i = 0; i < arraySize; ++i) {
    objects[i] = new Object();
  }

  cout << "About to allocate " << objects[10] << " objects" << endl;

  // Deallocated all the objects:
  delete objects[10];
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
}
