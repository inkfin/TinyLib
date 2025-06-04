#ifndef SIMPLE_MEMPOOL_H
#define SIMPLE_MEMPOOL_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

typedef uint8_t byte_t;

#define MEMPOOL_TOTAL_SIZE 1024 // Size of the memory pool in bytes

typedef struct memblock {
    struct memblock* next;
} memblock_t;

typedef struct mempool {
    memblock_t*   free_list;
    byte_t pool[MEMPOOL_TOTAL_SIZE];
} mempool_t;

#ifdef __cplusplus
extern "C" {
#endif

void mp_init(mempool_t* pool);
void* mp_alloc(mempool_t* pool);
void mp_free(mempool_t* pool, void* ptr);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_MEMPOOL_H

#ifdef TINY_MEMPOOL_IMPL

#ifdef __cplusplus
extern "C" {
#endif

void mp_init(mempool_t* pool)
{
    pool->free_list = (memblock_t*)pool->pool;

    memblock_t* current = pool->free_list;
    for (int i = 0; i < (int)(MEMPOOL_TOTAL_SIZE / sizeof(memblock_t)) - 1; i++) {
        current->next
            = (memblock_t*)((byte_t*)current + sizeof(memblock_t));
        current = current->next;
    }

    current->next = NULL; // Last block points to NULL
}

void* mp_alloc(mempool_t* pool)
{
    if (pool->free_list == NULL) {
        fprintf(stderr, "Memory pool exhausted\n");
        return NULL; // No free blocks available
    }

    memblock_t* block = pool->free_list;
    pool->free_list = block->next; // Move the free list pointer

    return (void*)block; // Return the allocated block
}

void mp_free(mempool_t* pool, void* ptr)
{
    if (!ptr)
        return; // Nothing to free

    memblock_t* block = (memblock_t*)ptr;
    block->next = pool->free_list; // Add the block back to the free list
    pool->free_list = block; // Update the free list pointer
}

#ifdef __cplusplus
}
#endif

#endif // TINY_MEMPOOL_IMPL
