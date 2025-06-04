#define TINY_MEMPOOL_IMPL
#include "tinylib/simple_mempool.h"

int main(void) {
    puts("==========================");
    puts("Tiny Memory Pool Test");

    mempool_t pool;
    mp_init(&pool);
    puts("Memory pool initialized.");

    void* block1 = mp_alloc(&pool);
    if (block1) {
        fprintf(stdout, "Allocated first block: %p; freelist=%p\n", block1, pool.free_list);
    } else {
        puts("Failed to allocate first block.");
    }

    void* block2 = mp_alloc(&pool);
    if (block2) {
        fprintf(stdout, "Allocated second block: %p; freelist=%p\n", block2, pool.free_list);
    } else {
        puts("Failed to allocate second block.");
    }

    mp_free(&pool, block1);
    fprintf(stdout, "Freed first block; free_list=%p\n", pool.free_list);

    mp_free(&pool, block2);
    fprintf(stdout, "Freed second block; free_list=%p\n", pool.free_list);

    puts("Memory pool test completed.");
    puts("==========================");
}
