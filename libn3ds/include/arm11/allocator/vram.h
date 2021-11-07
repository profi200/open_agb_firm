/**
 * @file vram.h
 * @brief VRAM allocator.
 */
#pragma once

/**
 * @brief Allocates a 0x80-byte aligned buffer.
 * @param size Size of the buffer to allocate.
 * @return The allocated buffer.
 */
void* vramAlloc(size_t size);

/**
 * @brief Allocates a buffer aligned to the given size.
 * @param size Size of the buffer to allocate.
 * @param alignment Alignment to use.
 * @return The allocated buffer.
 */
void* vramMemAlign(size_t size, size_t alignment);

/**
 * @brief Reallocates a buffer.
 * Note: Not implemented yet.
 * @param mem Buffer to reallocate.
 * @param size Size of the buffer to allocate.
 * @return The reallocated buffer.
 */
void* vramRealloc(void* mem, size_t size);

/**
 * @brief Retrieves the allocated size of a buffer.
 * @return The size of the buffer.
 */
size_t vramGetSize(void* mem);

/**
 * @brief Frees a buffer.
 * @param mem Buffer to free.
 */
void vramFree(void* mem);

/**
 * @brief Gets the current VRAM free space.
 * @return The current VRAM free space.
 */
u32 vramSpaceFree(void);
