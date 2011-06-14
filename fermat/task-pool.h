/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_TASK_POOL_H__
#define __FER_TASK_POOL_H__

#include <pthread.h>
#include <semaphore.h>
#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Task Pool - Pool of Threads
 * ============================
 *
 * See fer_task_pool_t.
 */

struct _fer_task_pool_t;

/**
 * Info about thread.
 */
struct _fer_task_pool_thinfo_t {
    int id; /*!< ID of thread - is same as index in .threads[] member of
                 fer_task_pool_t struct */
};
typedef struct _fer_task_pool_thinfo_t fer_task_pool_thinfo_t;


struct _fer_task_pool_thread_t {
    fer_task_pool_thinfo_t info; /*!< Thread info */
    struct _fer_task_pool_t *task_pool; /*!< Back reference to main struct */
    pthread_t th;                /*!< POSIX thread */
    fer_list_t tasks;            /*!< List of pending tasks */

    pthread_mutex_t lock;        /*!< Global lock for thread synchronization */
    sem_t full, empty;           /*!< Full/Empty semaphores for pending tasks */

    int pending;                 /*!< Number of pending tasks */
    pthread_cond_t pending_cond; /*!< Conditional variable to allow user
                                      code to wait until all tasks are
                                      finished */
};
typedef struct _fer_task_pool_thread_t fer_task_pool_thread_t;


struct _fer_task_pool_t {
    fer_task_pool_thread_t **threads; /*!< Array of threads */
    size_t threads_len;               /*!< Number of .threads array */
    int started;                      /*!< Set to 1 if all threads were
                                           started */
};
typedef struct _fer_task_pool_t fer_task_pool_t;



/**
 * Callback used as task
 */
typedef void (*fer_task_pool_fn)(int id, void *data,
                                 const fer_task_pool_thinfo_t *thinfo);

/**
 * Creates task pool containing {num_threads} threads.
 * Note that ferTaskPoolRun() must be called in order to actually start the
 * threads.
 */
fer_task_pool_t *ferTaskPoolNew(size_t num_threads);

/**
 * Waits for all tasks to be processed and then deletes task pool.
 */
void ferTaskPoolDel(fer_task_pool_t *t);

/**
 * Returns number of threads in task pool.
 */
_fer_inline size_t ferTaskPoolSize(const fer_task_pool_t *t);

/**
 * Adds task to {tid}'th thread in pool.
 * Returns 0 on success.
 */
void ferTaskPoolAdd(fer_task_pool_t *t, int tid,
                    fer_task_pool_fn fn, int id, void *data);

/**
 * Start all threads in pool.
 */
void ferTaskPoolRun(fer_task_pool_t *t);

/**
 * Returns number of pending tasks of {id}'th thread.
 */
int ferTaskPoolPending(fer_task_pool_t *t, int id);

/**
 * Blocks until all tasks of {id}'th thread aren't finished
 */
void ferTaskPoolBarrier(fer_task_pool_t *t, int id);

/**** INLINES ****/
_fer_inline size_t ferTaskPoolSize(const fer_task_pool_t *t)
{
    return t->threads_len;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_TASK_POOL_H__ */

