#pragma once


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
