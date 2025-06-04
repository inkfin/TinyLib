#ifndef TINYLIB_THREADS_WRAPPER_H
#define TINYLIB_THREADS_WRAPPER_H

#if !defined(TL_MALLOC) && !defined(TL_FREE)
#include <stdlib.h>
#define TL_MALLOC(size) malloc(size)
#define TL_FREE(ptr) free(ptr)
#elif !defined(TL_MALLOC) || !defined(TL_FREE)
#error "Please define TL_MALLOC and TL_FREE before including this header."
#endif

#if defined(__STDC_NO_THREADS__) || defined(__APPLE__)
#include <pthread.h>
#include <sched.h>
#include <time.h>

typedef pthread_t       tl_thrd_t;
typedef pthread_mutex_t tl_mtx_t;
typedef pthread_cond_t  tl_cnd_t;
typedef int             (*tl_thrd_start_t)(void*);

#define TL_THREAD_SUCCESS 0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    tl_thrd_start_t func;
    void*           arg;
    int             result;
} tl__thrd_ctx_t;

static void* tl__thrd_trampoline(void* p)
{
    tl__thrd_ctx_t* ctx = (tl__thrd_ctx_t*)p;
    ctx->result = ctx->func(ctx->arg);
    return NULL;
}

static inline int tl_thrd_create(
    tl_thrd_t* thr, tl_thrd_start_t func, void* arg)
{
    tl__thrd_ctx_t* ctx = TL_MALLOC(sizeof(tl__thrd_ctx_t));
    if (!ctx)
        return -1;
    ctx->func = func;
    ctx->arg = arg;
    ctx->result = 0;
    int ret = pthread_create(thr, NULL, tl__thrd_trampoline, ctx);
    if (ret != 0) {
        TL_FREE(ctx);
        return ret;
    }
    return 0;
}

static inline int tl_thrd_join(tl_thrd_t thr, int* out_result)
{
    void* retval;
    int   ret = pthread_join(thr, &retval);
    if (ret != 0)
        return ret;

    if (out_result) {
        tl__thrd_ctx_t* ctx = (tl__thrd_ctx_t*)retval;
        *out_result = ctx->result;
        free(ctx);
    }
    return 0;
}

static inline void tl_thrd_yield(void) { sched_yield(); }

static inline void tl_thrd_sleep(int millis)
{
    struct timespec ts = { millis / 1000, (millis % 1000) * 1000000 };
    nanosleep(&ts, NULL);
}

static inline int tl_mtx_init(tl_mtx_t* mtx)
{
    return pthread_mutex_init(mtx, NULL);
}

static inline int tl_mtx_destroy(tl_mtx_t* mtx)
{
    return pthread_mutex_destroy(mtx);
}

static inline int tl_mtx_lock(tl_mtx_t* mtx) { return pthread_mutex_lock(mtx); }

static inline int tl_mtx_unlock(tl_mtx_t* mtx)
{
    return pthread_mutex_unlock(mtx);
}

static inline int tl_cnd_init(tl_cnd_t* cnd)
{
    return pthread_cond_init(cnd, NULL);
}

static inline int tl_cnd_destroy(tl_cnd_t* cnd)
{
    return pthread_cond_destroy(cnd);
}

static inline int tl_cnd_wait(tl_cnd_t* cnd, tl_mtx_t* mtx)
{
    return pthread_cond_wait(cnd, mtx);
}

static inline int tl_cnd_signal(tl_cnd_t* cnd)
{
    return pthread_cond_signal(cnd);
}

static inline int tl_cnd_broadcast(tl_cnd_t* cnd)
{
    return pthread_cond_broadcast(cnd);
}

#ifdef __cplusplus
}
#endif

#else // __STDC_NO_THREADS__

// ======== C11 threads =========
#include <threads.h>
#include <time.h>

typedef thrd_t tl_thrd_t;
typedef mtx_t  tl_mtx_t;
typedef cnd_t  tl_cnd_t;
typedef int    (*tl_thrd_start_t)(void*);
#define TL_THREAD_SUCCESS thrd_success

#define tl_thrd_create thrd_create
#define tl_thrd_join(thr, res) thrd_join(thr, res)
#define tl_thrd_yield thrd_yield

static inline void tl_thrd_sleep(int millis)
{
    struct timespec ts = { millis / 1000, (millis % 1000) * 1000000 };
    thrd_sleep(&ts, NULL);
}

#define tl_mtx_init(mtx) mtx_init(mtx, mtx_plain)
#define tl_mtx_destroy mtx_destroy
#define tl_mtx_lock mtx_lock
#define tl_mtx_unlock mtx_unlock

#define tl_cnd_init cnd_init
#define tl_cnd_destroy cnd_destroy
#define tl_cnd_wait cnd_wait
#define tl_cnd_signal cnd_signal
#define tl_cnd_broadcast cnd_broadcast

#endif // __STDC_NO_THREADS__

#endif // TINYLIB_THREADS_WRAPPER_H
