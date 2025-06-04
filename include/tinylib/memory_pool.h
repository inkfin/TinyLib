/// memory_pool.h
///
/// Tiny C Lib memory pool implementation
/// references:
/// - <https://github.com/userpro/MemoryPool>
///

#ifndef TINYLIB_MEMORY_POOL_H
#define TINYLIB_MEMORY_POOL_H

#include <stddef.h>
#include <stdatomic.h>

/* Configurations */
#ifndef TINYLIB_MEMORY_POOL_NUM
#define TINYLIB_MEMORY_POOL_NUM 64
#endif

/* Declarations */

typedef struct tl_memchunk {
    size_t alloc_size;
    int    is_free; ///< 1 if free, 0 if allocated

    _Atomic(struct tl_memchunk*) prev, next;
} tl_memchunk_t;

typedef struct tl_mempool_list {
    char*    start; ///< pointer to the start of the memory pool
    uint32_t id; ///< memory pool id
    size_t   mempool_size;
    size_t   chunk_size;
    size_t   alloc_size; ///< size of the allocated memory
    size_t   alloc_prog_size; ///< size of the actual memory allocated for
                            ///< the program (excluding metadata)

    _Atomic(tl_memchunk_t*) alloc_list; ///< current slot for allocation
    _Atomic(tl_memchunk_t*) free_list; ///< free slots in the block

    _Atomic(struct tl_mempool_list*) next; ///< pointer to the next
                                           ///< memory pool in the list
} tl_mempool_list_t;

typedef struct tl_mempool {
    uint32_t     last_id; ///< last memory pool id
    uint32_t     auto_extend; ///< 1 if auto extend, 0 if not
    size_t       mempool_size; ///< size of the memory pool
    const size_t max_mempool_size; ///< maximum size of the memory pool
size_t alloc_mempool_size; ///< size of the allocated memory pool
    tl_mempool_list_t* mempool_list[TINYLIB_MEMORY_POOL_NUM]; ///< list of memory pools

    _Atomic(tl_mempool_list_t*) head; ///< pointer to the head of the memory pool list
    _Atomic(tl_mempool_list_t*) tail; ///< pointer to the tail of the memory pool list
} tl_mempool_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate memory for the memory pool
 * @param size size of the memory to allocate
 * @return pointer to the allocated memory
 */
void* tl_mempool_alloc(size_t size);

/**
 * @brief Allocate memory for the memory pool
 * @return 0 if success
 */
int tl_mempool_init(void);

/**
 * @brief Get the memory pool
 * @param idx index of the memory pool
 * @return pointer to the memory pool
 */
tl_mempool_list_t* tl_get_mempool(int idx);

void* tl_use_memory(size_t size);

void tl_free_memory(void* ptr, size_t size);

/**
 * @brief Free the memory pool
 * @param ptr pointer to the memory pool
 * @return 0 if success
 */
int tl_mempool_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_MEMORY_POOL_H

#ifdef TINYLIB_MEMORY_POOL_IMPL
#undef TINYLIB_MEMORY_POOL_IMPL

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

int tl_mempool_init(void) { return 0; }

void* tl_mempool_alloc(size_t size) { return NULL; }

int tl_mempool_free(void* ptr) { return -1; }

#ifdef __cplusplus
}
#endif

#endif // TINYLIB_MEMORY_POOL_IMPL
