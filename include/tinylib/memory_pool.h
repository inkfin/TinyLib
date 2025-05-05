/// dyn_arr.c
///
/// Tiny C Lib memory pool implementation
///

#ifndef TINYLIB_MEMORY_POOL_H
#define TINYLIB_MEMORY_POOL_H

#include <stdatomic.h>
#include <stdlib.h>
#include <threads.h>

/* Configurations */
#define TINYLIB_MEMORY_POOL_NUM 64
#define TINYLIB_MEM_SLOT_BASE_SIZE 8
#define TINYLIB_MEM_SLOT_MAX_SIZE 512

/* Declarations */
#ifdef __cplusplus
extern "C" {
#endif

struct tl_memslot_t;

typedef struct tl_memslot_t {
    _Atomic(struct tl_memslot*) next;
} tl_memslot_t;

typedef struct {
    size_t m_block_size;
    size_t m_slot_size;
    /// first slot in the block
    tl_memslot_t* m_first_slot;
    /// current slot in the block
    tl_memslot_t* m_curr_slot;
    /// free slots in the block
    _Atomic(tl_memslot_t*) m_free_slots;
    /// last slot in the block
    /// (reallocate required if m_curr_slot >= m_last_slots)
    tl_memslot_t* m_last_slots;
    mtx_t _mutex_block;
} tl_mem_pool_t;

///
/// Allocate memory for the memory pool
/// @param size size of the memory to allocate
/// @return pointer to the allocated memory
///
void* tl_mem_pool_alloc(size_t size);

///
/// Allocate memory for the memory pool
/// @return 0 if success
///
int tl_mem_pool_init(void);

///
/// Get the memory pool
/// @param idx index of the memory pool
/// @return pointer to the memory pool
///
tl_mem_pool_t* tl_get_mem_pool(int idx);

void* tl_use_memory(size_t size);

void tl_free_memory(void* ptr, size_t size);

///
/// Free the memory pool
/// @param ptr pointer to the memory pool
/// @return 0 if success
///
int tl_mem_pool_free(void* ptr);

#define TINYLIB_MEMORY_POOL_IMPL // TODO: COMMENT THIS
#ifdef TINYLIB_MEMORY_POOL_IMPL
#undef TINYLIB_MEMORY_POOL_IMPL

void* tl_mem_pool_alloc(size_t size)
{
    for (int i = 0; i < TINYLIB_MEMORY_POOL_NUM; ++i) {
        tl_mem_pool_t* pool = &g_mem_pools[i];
        if (pool->m_block_size == 0) {
            continue;
        }
        if (size <= pool->m_block_size) {
            tl_memslot_t* slot = atomic_exchange(&pool->m_free_slots, NULL);
            if (slot != NULL) {
                return slot;
            }
        }
    }
    return NULL;
}

int tl_mem_pool_free(void* ptr)
{
    return -1;
}

#endif // TINYLIB_MEMORY_POOL_IMPL

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_MEMORY_POOL_H
