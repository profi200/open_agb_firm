#pragma once
#include "types.h"
#include <stdlib.h>

struct MemChunk
{
	u8* addr;
	u32 size;
};

struct MemBlock
{
	MemBlock *prev, *next;
	u8* base;
	u32 size;

	static MemBlock* Create(u8* base, u32 size)
	{
		auto b = (MemBlock*)malloc(sizeof(MemBlock));
		if (!b) return nullptr;
		b->prev = nullptr;
		b->next = nullptr;
		b->base = base;
		b->size = size;
		return b;
	}
};

struct MemPool
{
	MemBlock *first, *last;

	bool Ready() { return first != nullptr; }

	void AddBlock(MemBlock* blk)
	{
		blk->prev = last;
		if (last) last->next = blk;
		if (!first) first = blk;
		last = blk;
	}

	void DelBlock(MemBlock* b)
	{
		auto prev = b->prev, &pNext = prev ? prev->next : first;
		auto next = b->next, &nPrev = next ? next->prev : last;
		pNext = next;
		nPrev = prev;
		free(b);
	}

	void InsertBefore(MemBlock* b, MemBlock* p)
	{
		auto prev = b->prev, &pNext = prev ? prev->next : first;
		b->prev = p;
		p->next = b;
		p->prev = prev;
		pNext = p;
	}

	void InsertAfter(MemBlock* b, MemBlock* n)
	{
		auto next = b->next, &nPrev = next ? next->prev : last;
		b->next = n;
		n->prev = b;
		n->next = next;
		nPrev = n;
	}

	//void CoalesceLeft(MemBlock* b);
	void CoalesceRight(MemBlock* b);

	bool Allocate(MemChunk& chunk, u32 size, int align);
	void Deallocate(const MemChunk& chunk);

	void Destroy()
	{
		MemBlock* next = nullptr;
		for (auto b = first; b; b = next)
		{
			next = b->next;
			free(b);
		}
		first = nullptr;
		last = nullptr;
	}

	//void Dump(const char* title);
	u32 GetFreeSpace();
};
