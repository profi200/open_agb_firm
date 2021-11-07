/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <stdlib.h>
#include "internal/slabheap.h"
#include "internal/kmemcpy_set.h"



void slabInit(SlabHeap *slab, size_t objSize, size_t num)
{
	if(objSize < sizeof(SlabHeap) || !num) return;

	listInit(slab);

	void *pool = malloc(objSize * num);
	if(!pool) return;
	do
	{
		listPush(slab, (SlabHeap*)pool);
		pool += objSize;
	} while(--num);
}

void* slabAlloc(SlabHeap *slab)
{
	if(!slab || listEmpty(slab)) return NULL;

	return listPop(slab);
}

void* slabCalloc(SlabHeap *slab, size_t clrSize)
{
	void *const ptr = slabAlloc(slab);
	if(ptr) kmemset(ptr, 0, clrSize);
	return ptr;
}

void slabFree(SlabHeap *slab, void *ptr)
{
	if(!slab || !ptr) return;

	// Keep gaps filled by allocating the same mem
	// again next time an object is allocated.
	listPushTail(slab, (SlabHeap*)ptr);
}
