#pragma once

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
#include "internal/list.h"


typedef ListNode SlabHeap;



/**
 * @brief      Initializes the slabheap.
 *
 * @param      slab     SlabHeap object pointer.
 * @param[in]  objSize  The size of the object slots.
 * @param[in]  num      The maximum number of object slots.
 */
void slabInit(SlabHeap *slab, size_t objSize, size_t num);

/**
 * @brief      Allocates an object slot from the slabheap.
 *
 * @param      slab  SlabHeap object pointer.
 *
 * @return     Returns a pointer to the object slot.
 */
void* slabAlloc(SlabHeap *slab);

/**
 * @brief      Same as slabAlloc() but clears slots.
 *
 * @param      slab     SlabHeap object pointer.
 * @param[in]  clrSize  The clear size (passed to memset()).
 *
 * @return     Returns a pointer to the object slot.
 */
void* slabCalloc(SlabHeap *slab, size_t clrSize);

/**
 * @brief      Deallocates an object slot.
 *
 * @param      slab  SlabHeap object pointer.
 * @param      ptr   The object slot pointer.
 */
void slabFree(SlabHeap *slab, void *ptr);
