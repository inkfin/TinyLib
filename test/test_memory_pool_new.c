// file: tl_mempool_test.c
// tinylib/TL memory pool tester
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>
#include <pthread.h>

// ======================= 配置区 =======================
// 逻辑参数（可按需调整）
#ifndef TL_POOL_BYTES
#define TL_POOL_BYTES (64ULL * 1024 * 1024) // 内存池总容量 64MB
#endif
#ifndef TL_CHUNK_SIZE
#define TL_CHUNK_SIZE 256 // 单块大小
#endif
#ifndef TL_ALIGNMENT
#define TL_ALIGNMENT 16 // 对齐
#endif
#ifndef TL_NUM_SLOTS
#define TL_NUM_SLOTS 4096 // 发布槽位个数（读写对抗测试用）
#endif
#ifndef TL_NUM_WRITERS
#define TL_NUM_WRITERS 4
#endif
#ifndef TL_NUM_READERS
#define TL_NUM_READERS 4
#endif
#ifndef TL_SECONDS
#define TL_SECONDS 5 // 压测时长（秒）
#endif
#ifndef TL_RAND_SEED
#define TL_RAND_SEED 0xC0FFEEu // 固定种子便于复现（改为time(NULL)可随机）
#endif

// ============ API 适配区（把这些宏改成你的真实API） ============
// 例：如果你的接口叫 mp_create/mp_alloc/mp_free/mp_destroy，改成：
//   #define TL_POOL_CREATE(bytes, chunk, align)  mp_create(bytes, chunk, align)
//   #define TL_POOL_ALLOC(pool)                  mp_alloc(pool)
//   #define TL_POOL_FREE(pool, ptr)              mp_free(pool, ptr)
//   #define TL_POOL_DESTROY(pool)                mp_destroy(pool)

void* tl_pool_create(size_t pool_bytes, size_t chunk_size, size_t alignment); // 声明，链接到你的库
void  tl_pool_destroy(void* pool);
void* tl_pool_alloc(void* pool);
void  tl_pool_free(void* pool, void* p);

#define TL_POOL_CREATE(bytes, chunk, align) tl_pool_create((bytes), (chunk), (align))
#define TL_POOL_ALLOC(pool) tl_pool_alloc((pool))
#define TL_POOL_FREE(pool, ptr) tl_pool_free((pool), (ptr))
#define TL_POOL_DESTROY(pool) tl_pool_destroy((pool))

// =================== 校验结构与工具 ===================
typedef struct TLBlockHeader {
    uint32_t magic; // 魔数
    uint32_t writer_id; // 写线程ID
    uint64_t seq; // 递增序号
    uint32_t payload_sz; // 有效负载长度（不含头）
    uint32_t checksum; // 简单校验和
} TLBlockHeader;

#define TL_MAGIC 0x544C4248u /* 'T''L''B''H' */

static inline uint32_t tl_checksum(const uint8_t* p, size_t n)
{
    uint32_t s = 0x9E3779B9u;
    for (size_t i = 0; i < n; ++i) {
        s = (s << 5) ^ (s >> 2) ^ p[i];
    }
    // final mix
    s ^= s >> 16;
    s *= 0x7feb352dU;
    s ^= s >> 15;
    s *= 0x846ca68bU;
    s ^= s >> 16;
    return s;
}

static inline void tl_fill_block(void* mem, size_t chunk_size, uint32_t writer_id, uint64_t seq)
{
    TLBlockHeader* h = (TLBlockHeader*)mem;
    h->magic = TL_MAGIC;
    h->writer_id = writer_id;
    h->seq = seq;
    size_t payload = (chunk_size > sizeof(TLBlockHeader)) ? (chunk_size - sizeof(TLBlockHeader)) : 0;
    if (payload) {
        uint8_t* data = (uint8_t*)(h + 1);
        // 用可复现pattern填充
        for (size_t i = 0; i < payload; ++i) {
            data[i] = (uint8_t)((seq + i + writer_id) & 0xFF);
        }
        h->payload_sz = (uint32_t)payload;
        h->checksum = tl_checksum(data, payload);
    } else {
        h->payload_sz = 0;
        h->checksum = 0;
    }
}

static inline int tl_check_block(const void* mem, size_t chunk_size)
{
    const TLBlockHeader* h = (const TLBlockHeader*)mem;
    if (h->magic != TL_MAGIC)
        return -1;
    if (h->payload_sz + sizeof(TLBlockHeader) > chunk_size)
        return -2;
    const uint8_t* data = (const uint8_t*)(h + 1);
    uint32_t       cs = tl_checksum(data, h->payload_sz);
    return (cs == h->checksum) ? 0 : -3;
}

// =================== 发布槽（读写对抗） ===================

// writers 将块放入 slots，readers 取走并校验后归还
static _Atomic(uintptr_t) g_slots[TL_NUM_SLOTS];

typedef struct ThreadStat {
    uint64_t ops_ok;
    uint64_t ops_fail;
} ThreadStat;

typedef struct WriterCtx {
    void*                 pool;
    uint32_t              id;
    size_t                chunk_size;
    atomic_uint_fast64_t* global_seq;
    ThreadStat            stat;
} WriterCtx;

typedef struct ReaderCtx {
    void*      pool;
    size_t     chunk_size;
    ThreadStat stat;
} ReaderCtx;

static inline uint32_t fast_rand(uint32_t* state)
{
    // xorshift32
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void* writer_thread(void* arg)
{
    WriterCtx* ctx = (WriterCtx*)arg;
    uint32_t   rng = TL_RAND_SEED ^ (0xA5A5A5A5u + ctx->id);
    time_t     t_end = time(NULL) + TL_SECONDS;

    while (time(NULL) < t_end) {
        void* p = TL_POOL_ALLOC(ctx->pool);
        if (!p) {
            ctx->stat.ops_fail++;
            continue;
        }

        uint64_t seq = atomic_fetch_add_explicit(ctx->global_seq, 1, memory_order_relaxed);
        tl_fill_block(p, ctx->chunk_size, ctx->id, seq);

        // 随机挑一个槽位发布
        uint32_t  idx = fast_rand(&rng) % TL_NUM_SLOTS;
        uintptr_t want = 0;
        if (atomic_compare_exchange_strong(&g_slots[idx], &want, (uintptr_t)p)) {
            ctx->stat.ops_ok++;
        } else {
            // 槽位被占，直接释放（模拟写失败时回收）
            TL_POOL_FREE(ctx->pool, p);
            ctx->stat.ops_fail++;
        }
    }
    return NULL;
}

static void* reader_thread(void* arg)
{
    ReaderCtx* ctx = (ReaderCtx*)arg;
    uint32_t   rng = TL_RAND_SEED ^ 0x5A5A5A5Au;
    time_t     t_end = time(NULL) + TL_SECONDS;

    while (time(NULL) < t_end) {
        uint32_t  idx = fast_rand(&rng) % TL_NUM_SLOTS;
        uintptr_t got = atomic_exchange(&g_slots[idx], (uintptr_t)0);
        if (!got) {
            continue;
        }

        void* p = (void*)got;
        int   ok = tl_check_block(p, ctx->chunk_size);
        if (ok == 0) {
            ctx->stat.ops_ok++;
        } else {
            ctx->stat.ops_fail++;
            fprintf(stderr, "[reader] data corrupt: code=%d\n", ok);
        }
        TL_POOL_FREE(ctx->pool, p);
    }
    return NULL;
}

// =================== 基础单线程测试 ===================
static int single_thread_sanity(void* pool, size_t chunk)
{
    enum { N = 10000 };
    for (int i = 0; i < N; ++i) {
        void* p = TL_POOL_ALLOC(pool);
        if (!p) {
            fprintf(stderr, "sanity: alloc failed at %d\n", i);
            return -1;
        }
        tl_fill_block(p, chunk, /*writer_id=*/123, /*seq=*/(uint64_t)i);
        if (tl_check_block(p, chunk) != 0) {
            fprintf(stderr, "sanity: verify failed at %d\n", i);
            return -2;
        }
        TL_POOL_FREE(pool, p);
    }
    // 重复利用测试
    void* a = TL_POOL_ALLOC(pool);
    void* b = TL_POOL_ALLOC(pool);
    TL_POOL_FREE(pool, a);
    void* c = TL_POOL_ALLOC(pool); // 这里应能复用 a 的块（无法直接断言，但至少不崩）
    TL_POOL_FREE(pool, b);
    TL_POOL_FREE(pool, c);
    return 0;
}

// =================== 主流程 ===================
int main(void)
{
    printf("[TL test] pool_bytes=%zu, chunk=%zu, align=%zu, slots=%d, "
           "writers=%d, readers=%d, seconds=%d\n",
        (size_t)TL_POOL_BYTES, (size_t)TL_CHUNK_SIZE, (size_t)TL_ALIGNMENT, TL_NUM_SLOTS, TL_NUM_WRITERS,
        TL_NUM_READERS, TL_SECONDS);

    // 清空槽位
    for (int i = 0; i < TL_NUM_SLOTS; ++i)
        atomic_store(&g_slots[i], (uintptr_t)0);

    // 创建内存池
    void* pool = TL_POOL_CREATE(TL_POOL_BYTES, TL_CHUNK_SIZE, TL_ALIGNMENT);
    if (!pool) {
        fprintf(stderr, "failed to create pool\n");
        return 1;
    }

    // 单线程自检
    if (single_thread_sanity(pool, TL_CHUNK_SIZE) != 0) {
        fprintf(stderr, "single_thread_sanity failed\n");
        TL_POOL_DESTROY(pool);
        return 2;
    }
    puts("[sanity] ok");

    // 多线程读写对抗
    atomic_uint_fast64_t global_seq = 0;
    pthread_t            writers[TL_NUM_WRITERS];
    pthread_t            readers[TL_NUM_READERS];
    WriterCtx            wctx[TL_NUM_WRITERS];
    ReaderCtx            rctx[TL_NUM_READERS];

    for (int i = 0; i < TL_NUM_WRITERS; ++i) {
        wctx[i].pool = pool;
        wctx[i].id = (uint32_t)i;
        wctx[i].chunk_size = TL_CHUNK_SIZE;
        wctx[i].global_seq = &global_seq;
        wctx[i].stat = (ThreadStat) { 0, 0 };
        if (pthread_create(&writers[i], NULL, writer_thread, &wctx[i]) != 0) {
            perror("pthread_create writer");
            TL_POOL_DESTROY(pool);
            return 3;
        }
    }
    for (int i = 0; i < TL_NUM_READERS; ++i) {
        rctx[i].pool = pool;
        rctx[i].chunk_size = TL_CHUNK_SIZE;
        rctx[i].stat = (ThreadStat) { 0, 0 };
        if (pthread_create(&readers[i], NULL, reader_thread, &rctx[i]) != 0) {
            perror("pthread_create reader");
            TL_POOL_DESTROY(pool);
            return 4;
        }
    }

    for (int i = 0; i < TL_NUM_WRITERS; ++i)
        pthread_join(writers[i], NULL);
    for (int i = 0; i < TL_NUM_READERS; ++i)
        pthread_join(readers[i], NULL);

    // 汇总统计
    uint64_t w_ok = 0, w_fail = 0, r_ok = 0, r_fail = 0;
    for (int i = 0; i < TL_NUM_WRITERS; ++i) {
        w_ok += wctx[i].stat.ops_ok;
        w_fail += wctx[i].stat.ops_fail;
    }
    for (int i = 0; i < TL_NUM_READERS; ++i) {
        r_ok += rctx[i].stat.ops_ok;
        r_fail += rctx[i].stat.ops_fail;
    }
    printf("[summary] writers ok=%llu fail=%llu | readers ok=%llu fail=%llu | "
           "produced seq=%llu\n",
        (unsigned long long)w_ok, (unsigned long long)w_fail, (unsigned long long)r_ok, (unsigned long long)r_fail,
        (unsigned long long)atomic_load(&global_seq));

    // 检查是否有未消费的块，逐个取走并归还，避免泄漏
    uint64_t leaked = 0;
    for (int i = 0; i < TL_NUM_SLOTS; ++i) {
        uintptr_t p = atomic_exchange(&g_slots[i], (uintptr_t)0);
        if (p) {
            ++leaked;
            TL_POOL_FREE(pool, (void*)p);
        }
    }
    if (leaked)
        printf("[summary] drained %llu pending slots\n", (unsigned long long)leaked);

    TL_POOL_DESTROY(pool);
    puts("[done]");
    return (r_fail == 0) ? 0 : 5;
}
