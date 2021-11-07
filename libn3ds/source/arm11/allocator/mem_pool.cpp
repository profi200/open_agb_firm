#include "mem_pool.h"

/*
// This method is currently unused
void MemPool::CoalesceLeft(MemBlock* b)
{
	auto curPtr = b->base;
	for (auto p = b->prev; p; p = p->prev)
	{
		if ((p->base + p->size) != curPtr) break;
		curPtr = p->base;
		p->size += b->size;
		DelBlock(b);
		b = p;
	}
}
*/

void MemPool::CoalesceRight(MemBlock* b)
{
	auto curPtr = b->base + b->size;
	auto next = b->next;
	for (auto n = next; n; n = next)
	{
		next = n->next;
		if (n->base != curPtr) break;
		b->size += n->size;
		curPtr += n->size;
		DelBlock(n);
	}
}

bool MemPool::Allocate(MemChunk& chunk, u32 size, int align)
{
	// Don't shift out of bounds (CERT INT34-C)
	if(align >= 32 || align < 0)
		return false;

	// Alignment must not be 0
	if(align == 0)
		return false;

	u32 alignMask = (1 << align) - 1;

	// Check if size doesn't fit neatly in alignment
	if(size & alignMask)
	{
		// Make sure addition won't overflow (CERT INT30-C)
		if(size > UINT32_MAX - alignMask)
			return false;

		// Pad size to next alignment
		size = (size + alignMask) &~ alignMask;
	}

	// Find the first suitable block
	for (auto b = first; b; b = b->next)
	{
		auto addr = b->base;
		u32 begWaste = (u32)addr & alignMask;
		if (begWaste > 0) begWaste = alignMask + 1 - begWaste;
		if (begWaste > b->size) continue;
		addr += begWaste;
		u32 bSize = b->size - begWaste;
		if (bSize < size) continue;

		// Found space!
		chunk.addr = addr;
		chunk.size = size;

		// Resize the block
		if (!begWaste)
		{
			b->base += size;
			b->size -= size;
			if (!b->size)
				DelBlock(b);
		} else
		{
			auto nAddr = addr + size;
			auto nSize = bSize - size;
			b->size = begWaste;
			if (nSize)
			{
				// We need to add the tail chunk that wasn't used to the list
				auto n = MemBlock::Create(nAddr, nSize);
				if (n) InsertAfter(b, n);
				else   chunk.size += nSize; // we have no choice but to waste the space.
			}
		}
		return true;
	}

	return false;
}

void MemPool::Deallocate(const MemChunk& chunk)
{
	u8*  cAddr = chunk.addr;
	auto cSize = chunk.size;
	bool done = false;

	// Try to merge the chunk somewhere into the list
	for (auto b = first; !done && b; b = b->next)
	{
		auto addr = b->base;
		if (addr > cAddr)
		{
			if ((cAddr + cSize) == addr)
			{
				// Merge the chunk to the left of the block
				b->base = cAddr;
				b->size += cSize;
			} else
			{
				// We need to insert a new block
				auto c = MemBlock::Create(cAddr, cSize);
				if (c) InsertBefore(b, c);
			}
			done = true;
		} else if ((b->base + b->size) == cAddr)
		{
			// Coalesce to the right
			b->size += cSize;
			CoalesceRight(b);
			done = true;
		}
	}

	if (!done)
	{
		// Either the list is empty or the chunk address is past the end
		// address of the last block -- let's add a new block at the end
		auto b = MemBlock::Create(cAddr, cSize);
		if (b) AddBlock(b);
	}
}

/*
void MemPool::Dump(const char* title)
{
	printf("<%s> VRAM Pool Dump\n", title);
	for (auto b = first; b; b = b->next)
		printf("  - %p (%u bytes)\n", b->base, b->size);
}
*/

u32 MemPool::GetFreeSpace()
{
	u32 acc = 0;
	for (auto b = first; b; b = b->next)
		acc += b->size;
	return acc;
}
