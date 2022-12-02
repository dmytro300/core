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
		: _sz(std::max(sizeof(Chunk), sz_)), _chunksPerBlock(chunksPerBlock) , _blockSz(nextPowerTwo(_sz * chunksPerBlock))
	{
  	std::cout << "\nPoolAllocator sz:" << sz_ << " blockSize:" << _blockSz << std::endl;
	}

	void *allocate();
	void deallocate(void *ptr);

	private:

	const size_t _sz;
	const size_t _chunksPerBlock;
	const size_t _blockSz;

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
  auto res = posix_memalign(&blockBegin, _blockSz, _blockSz);

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
void PoolAllocator::deallocate(void *chunk) {

	if( !is_aligned(chunk,_sz) )
	{
  	std::cerr << "\nDeallocation of wrong  address." << std::endl;
		throw std::bad_alloc();
	}

	Chunk* ptr = reinterpret_cast<Chunk *>(align_back(chunk, _blockSz));

	if( std::find(chunks.begin(), chunks.end(), ptr) == chunks.end() )
	{
  	std::cerr << "\nDeallocation of wrong outside address." << std::endl;
		throw std::bad_alloc();
	}

  // The freed chunk's next pointer points to the
  // current allocation pointer:
  reinterpret_cast<Chunk *>(chunk)->next = mAlloc;

  // And the allocation pointer is now set
  // to the returned (free) chunk:

  mAlloc = reinterpret_cast<Chunk *>(chunk);
}
