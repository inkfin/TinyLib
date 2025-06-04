#define TINY_MEMPOOL_IMPL
#include "tinylib/tiny_mempool.h"

int main(void) {
    puts("==========================");
    puts("Tiny Memory Pool Test");

    mempool_t pool;
    init_mempool(&pool);
    puts("Memory pool initialized.");

    void* block1 = allocate_memory(&pool);
    if (block1) {
        fprintf(stdout, "Allocated first block: %p; freelist=%p\n", block1, pool.free_list);
    } else {
        puts("Failed to allocate first block.");
    }

    void* block2 = allocate_memory(&pool);
    if (block2) {
        fprintf(stdout, "Allocated second block: %p; freelist=%p\n", block2, pool.free_list);
    } else {
        puts("Failed to allocate second block.");
    }

    free_memory(&pool, block1);
    fprintf(stdout, "Freed first block; free_list=%p\n", pool.free_list);

    free_memory(&pool, block2);
    fprintf(stdout, "Freed second block; free_list=%p\n", pool.free_list);

    puts("Memory pool test completed.");
    puts("==========================");
}
