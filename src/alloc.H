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


/*
**
 * The allocator class.
 *
 * Features:
 *
 *   - Parametrized by number of Chunks per block
 *   - Keeps track of the allocation pointer
 *   - Bump-allocates Chunks
 *   - Requests a new larger block when needed
 *
 */
class PoolAllocator {

	struct Chunk {
		Chunk *next;
	};

	public:
	PoolAllocator(size_t sz_, size_t chunksPerBlock)
		: _sz(std::max(sizeof(Chunk), sz_)), _blockSz(_sz * chunksPerBlock), _chunksPerBlock (chunksPerBlock)
	{
  	std::cout << "PoolAllocator assumes" 
							<< " sz:" << _sz 
							<< " blockSize:" << _blockSz 
							<< " chunksPerBlock:" << _chunksPerBlock 
							<< std::endl;
	}

	void *allocate();
	void deallocate(void *ptr);

	private:

	const size_t _sz;
	const size_t _blockSz;
	const size_t _chunksPerBlock;

	/**
	 * Allocation pointer.
	 */
	Chunk *mAlloc = nullptr;
	std::vector<Chunk*> chunks;

	/**
	 * Allocates a larger block (pool) for Chunks.
	 */
	Chunk *allocateBlock(size_t chunkSize);
};

inline PoolAllocator::Chunk *PoolAllocator::allocateBlock(size_t chunkSize) 
{

  // The first Chunk of the new block.
  //Chunk *blockBegin = reinterpret_cast<Chunk *>(malloc(blockSize));
	void *blockBegin = nullptr;
  auto res = posix_memalign(&blockBegin, nextPowerTwo(_sz - 1), _blockSz);

	if(res)
	{
  	std::cerr << "\nFailed to allocate block." << std::endl;
		throw std::bad_alloc();
	}	

  std::cout << "\nAllocating block (" << blockBegin  << ")" << std::endl;

  Chunk *ch = reinterpret_cast<Chunk *>(blockBegin);
	chunks.push_back(ch);
	
  // Once the block is allocated, we need to chain all
  // the Chunks in this block:
  for (size_t i = 0; i < _chunksPerBlock - 1; ++i) {
    ch->next =
        reinterpret_cast<Chunk *>(reinterpret_cast<char *>(ch) + chunkSize);
    ch = ch->next;
  }

  ch->next = nullptr;

  return reinterpret_cast<Chunk *>(blockBegin);
}

/**
 * Returns the first free chunk in the block.
 *
 * If there are no chunks left in the block,
 * allocates a new block.
 */
void *PoolAllocator::allocate() {

  // No chunks left in the current block, or no any block
  // exists yet. Allocate a new one, passing the chunk size:

  std::cout << "alloc" << std::endl;

  if (mAlloc == nullptr) {
    mAlloc = allocateBlock(_sz);
  }

  // The return value is the current position of
  // the allocation pointer:
  Chunk *freeChunk = mAlloc;

  // Advance (bump) the allocation pointer to the next chunk.
  //
  // When no chunks left, the `mAlloc` will be set to `nullptr`, and
  // this will cause allocation of a new block on the next request:
  mAlloc = mAlloc->next;

  return freeChunk;
}

/**
 * Puts the chunk into the front of the chunks list.
 */
void PoolAllocator::deallocate(void *chunk) 
{
  std::cout << "dealloc" << std::endl;

	if( chunk == mAlloc )
	{
  	std::cerr << "Double free." << std::endl;
		//throw std::bad_alloc();
		return;
	}

	/*
	if( !is_aligned(chunk,_sz) )
	{
  	std::cerr << "Deallocation of wrong misaligned address." << std::endl;
		//throw std::bad_alloc();
		return;
	}
	*/

	Chunk* ptr = reinterpret_cast<Chunk *>(chunk);
	bool found = false;
	for(size_t i=0; i<chunks.size(); i++)
	{
		if( chunks[i] <= ptr && ptr <= chunks[i] + _blockSz )
		{
			found = true;
			break;
		}
	}

	if( !found )
	{
		std::cerr << "Deallocation of wrong outside address." << std::endl;
		//throw std::bad_alloc();
		return;
	}

  // The freed chunk's next pointer points to the
  // current allocation pointer:
  reinterpret_cast<Chunk *>(chunk)->next = mAlloc;

  // And the allocation pointer is now set
  // to the returned (free) chunk:

  mAlloc = reinterpret_cast<Chunk *>(chunk);
}

/*
template<class R, class...Args>
struct func<R(Args...)> {
  mutable std::any state;
  R(*f)(std::any& state, Args&&...) = nullptr;
  template<class T>
  void bind(T&& t) {
    state = std::forward<T>(t);
    f = [](std::any& state, Args&&...args)->R {
      return std::any_cast<T&>(state)(std::forward<Args>(args)...);
    };
  }
  R operator()(Args...args)const {
    return f(state, std::forward<Args>(args)...);
  }
};
*/

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
	: allocator(sizeof(Item),TABLE_SIZE*2)
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
				entry = new (allocator.allocate()) Item{key, value};
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


