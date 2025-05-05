#include "test.h"
#include "tinylib/memory_pool.h"
#include <stdio.h>
#include <threads.h>

#define newElement(type) (type*)tl_mem_pool_alloc(sizeof(type))
#define deleteElement(ptr) tl_mem_pool_free((void*)(ptr))

typedef struct {
    int id_;
} P1;
typedef struct {
    int id_[5];
} P2;
typedef struct {
    int id_[10];
} P3;
typedef struct {
    int id_[20];
} P4;

typedef struct {
    size_t ntimes;
    size_t rounds;
    size_t idx;
} thread_arg_t;

static size_t* pool_costs;
static size_t* malloc_costs;

int thread_pool_func(void* v)
{
    thread_arg_t* ta = v;
    size_t total = 0;
    for (size_t r = 0; r < ta->rounds; ++r) {
        clock_t t0 = clock();
        for (size_t i = 0; i < ta->ntimes; ++i) {
            P1* p1 = newElement(P1);
            deleteElement(p1);
            P2* p2 = newElement(P2);
            deleteElement(p2);
            P3* p3 = newElement(P3);
            deleteElement(p3);
            P4* p4 = newElement(P4);
            deleteElement(p4);
        }
        clock_t t1 = clock();
        total += (size_t)(t1 - t0);
    }
    pool_costs[ta->idx] = total;
    return 0;
}

int thread_malloc_func(void* v)
{
    thread_arg_t* ta = v;
    size_t total = 0;
    for (size_t r = 0; r < ta->rounds; ++r) {
        clock_t t0 = clock();
        for (size_t i = 0; i < ta->ntimes; ++i) {
            P1* p1 = malloc(sizeof *p1);
            free(p1);
            P2* p2 = malloc(sizeof *p2);
            free(p2);
            P3* p3 = malloc(sizeof *p3);
            free(p3);
            P4* p4 = malloc(sizeof *p4);
            free(p4);
        }
        clock_t t1 = clock();
        total += (size_t)(t1 - t0);
    }
    malloc_costs[ta->idx] = total;
    return 0;
}

void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
    thrd_t* threads = malloc(nworks * sizeof *threads);
    thread_arg_t* args = malloc(nworks * sizeof *args);
    pool_costs = malloc(nworks * sizeof *pool_costs);

    for (size_t k = 0; k < nworks; ++k) {
        args[k] = (thread_arg_t) { ntimes, rounds, k };
        thrd_create(&threads[k], thread_pool_func, &args[k]);
    }
    size_t total = 0;
    for (size_t k = 0; k < nworks; ++k) {
        thrd_join(threads[k], NULL);
        total += pool_costs[k];
    }
    printf("%lu threads × %lu rounds × %lu ops (pool) = %lu ticks\n",
        nworks, rounds, ntimes, total);

    free(threads);
    free(args);
    free(pool_costs);
}

void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds)
{
    thrd_t* threads = malloc(nworks * sizeof *threads);
    thread_arg_t* args = malloc(nworks * sizeof *args);
    malloc_costs = malloc(nworks * sizeof *malloc_costs);

    for (size_t k = 0; k < nworks; ++k) {
        args[k] = (thread_arg_t) { ntimes, rounds, k };
        thrd_create(&threads[k], thread_malloc_func, &args[k]);
    }
    size_t total = 0;
    for (size_t k = 0; k < nworks; ++k) {
        thrd_join(threads[k], NULL);
        total += malloc_costs[k];
    }
    printf("%lu threads × %lu rounds × %lu ops (malloc) = %lu ticks\n",
        nworks, rounds, ntimes, total);

    free(threads);
    free(args);
    free(malloc_costs);
}

void memory_pool_test_cases(void)
{
    puts("- Memory Pool Test Cases");
    tl_mem_pool_init();

    BenchmarkMemoryPool(100, 1, 10);
    puts("================================================================");
    BenchmarkNew       (100, 1, 10);
}
