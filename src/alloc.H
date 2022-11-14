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
	PoolAllocator(size_t chunksPerBlock)
		: _chunksPerBlock(chunksPerBlock) {}

	void *allocate(size_t size);
	void deallocate(void *ptr);

	private:

	size_t _chunksPerBlock;

	/**
	 * Allocation pointer.
	 */
	Chunk *mAlloc = nullptr;

	/**
	 * Allocates a larger block (pool) for Chunks.
	 */
	Chunk *allocateBlock(size_t chunkSize);
};

inline PoolAllocator::Chunk *PoolAllocator::allocateBlock(size_t chunkSize) 
{
  std::cout << "\nAllocating block (" << _chunksPerBlock << " Chunks):\n\n" << std::endl;

  size_t blockSize = _chunksPerBlock * chunkSize;

  // The first Chunk of the new block.
  Chunk *blockBegin = reinterpret_cast<Chunk *>(malloc(blockSize));

  Chunk *ch = blockBegin;

  // Once the block is allocated, we need to chain all
  // the Chunks in this block:

  for (size_t i = 0; i < _chunksPerBlock - 1; ++i) {
    ch->next =
        reinterpret_cast<Chunk *>(reinterpret_cast<char *>(ch) + chunkSize);
    ch = ch->next;
  }

  ch->next = nullptr;

  return blockBegin;
}

/**
 * Returns the first free chunk in the block.
 *
 * If there are no chunks left in the block,
 * allocates a new block.
 */
void *PoolAllocator::allocate(size_t size) {

  // No chunks left in the current block, or no any block
  // exists yet. Allocate a new one, passing the chunk size:

  if (mAlloc == nullptr) {
    mAlloc = allocateBlock(size);
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

  // The freed chunk's next pointer points to the
  // current allocation pointer:

  reinterpret_cast<Chunk *>(chunk)->next = mAlloc;

  // And the allocation pointer is now set
  // to the returned (free) chunk:

  mAlloc = reinterpret_cast<Chunk *>(chunk);
}
