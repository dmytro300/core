#pragma once

#include <vector>
#include <array>
#include <map>
#include <string>
#include <string.h>
#include <tuple>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

// compute the next highest power of 2 of 32-bit v
inline unsigned int nextPowerTwo(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline void*
align_back(void* ptr, std::size_t alignment) noexcept
{
    std::uintptr_t ptr_int = reinterpret_cast<std::uintptr_t>(ptr);
    std::size_t remainder = ptr_int % alignment;
    return static_cast<char*>(ptr) - remainder;
}

inline bool is_aligned(const void *pointer, size_t byte_count)
{ 
		return (uintptr_t)pointer % byte_count == 0; 
}


struct PoolAllocator {
	void *allocate(size_t sz_) { return ::malloc(sz_); }
	void deallocate(void *ptr) { free(ptr); }
};

template <typename K, typename V>
struct HashNode
{
    K key;
    V value;
    // next bucket with the same key
    HashNode *next = NULL;
};

template <typename K, typename V, unsigned int TABLE_SIZE=16>
class HashMap {
public:

	typedef HashNode<K, V> Item;
	typedef std::hash<K> H;

	HashMap() 
	{
			// construct zero initialized hash table of size
			table = new Item*[TABLE_SIZE]();
	}

	~HashMap() {
			// destroy all buckets one by one
			for (unsigned int i = 0; i < TABLE_SIZE; ++i) {
					Item *entry = table[i];
					while (entry != NULL) 
					{
							Item *prev = entry;
							entry = entry->next;
							prev->~Item();
							allocator.deallocate(prev);
					}
					table[i] = NULL;
			}
			// destroy the hash table
			delete [] table;
	}

	bool get(const K &key, V &value) 
	{
			unsigned long hashValue = H()(key) & (TABLE_SIZE - 1);

			Item *entry = table[hashValue];

			while (entry != NULL) {
					if (entry->key == key) {
							value = entry->value;
							return true;
					}
					entry = entry->next;
			}
			return false;
	}

	void put(const K &key, const V &value) 
	{
		unsigned long hashValue = H()(key) & (TABLE_SIZE - 1);
		Item *prev = NULL;
		Item *entry = table[hashValue];

		while (entry != NULL && entry->key != key) 
		{
				prev = entry;
				entry = entry->next;
		}

		if (entry == NULL) 
		{
				entry = new (allocator.allocate(sizeof(Item))) Item{key, value};
				if (prev == NULL) {
						// insert as first bucket
						table[hashValue] = entry;
				} else {
						prev->next = entry;
				}
		} else {
				// just update the value
				entry->value = value;
		}
	}

	void remove(const K &key) 
	{
		unsigned long hashValue = H()(key) & (TABLE_SIZE - 1);

		Item *prev = NULL;
		Item *entry = table[hashValue];

		while (entry != NULL && entry->key != key) {
				prev = entry;
				entry = entry->next;
		}

		if (entry == NULL) {
				// key not found
				return;
		}
		else {
				if (prev == NULL) {
						// remove first bucket of the list
						table[hashValue] = entry->next;
				} else {
						prev->next = entry->next;
				}
				entry->~Item();
				allocator.deallocate(entry);
		}
	}

private:
    // hash table
    Item** table;
  	PoolAllocator allocator;
};

template<typename T, size_t S=16> 
struct fixed_vector
{
	typedef T& ref;
	typedef const T& cref;
	typedef T* ptr;

	std::array<T,S> _v; 
	size_t  _size = 0;

	constexpr ref operator[]( size_t pos ){
		return _v[pos];
	}

  template <typename... Args>
  constexpr ref emplace_back(Args&&... args) {
		if (_size == S) throw std::bad_alloc();
    return *new (_v.data() + _size++) T(std::forward<Args>(args)...);
  }

  constexpr ref push_back(const T & t) { return emplace_back(t); }
  constexpr ref push_back(T && t) { return emplace_back(std::move(t)); }	

	constexpr ptr begin() { return _v.begin(); }
	constexpr ptr end() { return _v.end(); }

	constexpr size_t size()const{ return _size; }
};


