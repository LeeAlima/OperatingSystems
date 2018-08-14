/*
 * Lee Alima
 * 313467441
 */

#ifndef EX4_THREADPOOL_H
#define EX4_THREADPOOL_H

#include <sys/types.h>
#include "osqueue.h"
#include <stdlib.h>
#include <pthread.h>

/**
 * ThreadPool status
 * RunningNormally -> threadPool was initialize and is running
 * StopAndContinueExecuting -> should stop after finis running the
 * running tasks
 * Stopped -> threadPool was destroy and should be close after
 * running all the tasks in the queue
 */
enum Status{RunningNormally, StopAndContinueExecuting, Stopped};

/**
 * Saving struct represents a threadPool
 */
typedef struct thread_pool
{
    int size; // the pool size
    OSQueue* tasksQueue; // tasks queue
    pthread_t* threads; // dynamic array of threads
    enum Status status; // threadPool status
    pthread_mutex_t mutex; // mutex to lock the critical sections
    pthread_cond_t handleBusyWaiting; // cond to handle busy waiting
}ThreadPool;

/**
 * Struct to save a task
 * saving a pointer to a funcion and parameters
 */
typedef struct func
{
    void(*computeFunc)(void*);
    void* param;
}Func;


ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif //EX4_THREADPOOL_H
