/*
 * Lee Alima
 * 313467441
 */

#include <unistd.h>
#include "threadPool.h"

#define FAIL -1
#define STDERR 2
#define LENGTH_ERROR_MSG 21
#define COMPLETE 0


void* executeTasks(void* threadPoolVoid);
void freeThreadPool(ThreadPool *threadPool);
void handleFailure();

/**
 * This function is responsible of allocating and initializing a new
 * ThreadPool struct and its feilds.
 * @param numOfThreads - number of threads in the pool
 * @return ptr to the ThreadPool struct
 */
ThreadPool* tpCreate(int numOfThreads){
    int i,check;
    ThreadPool* pool = malloc(sizeof(ThreadPool));
    if (!pool){ // If malloc failed
        handleFailure();
    }
    pool->size = numOfThreads;
    // Allocate dynamic array of threads
    pool->threads = malloc(numOfThreads*sizeof(pthread_t));
    if (!pool->threads){ // If malloc failed
        free(pool);
        handleFailure();
    }
    pool->tasksQueue = osCreateQueue();
    if (!pool->tasksQueue){ // If malloc failed
        free(pool->threads);
        free(pool);
        handleFailure();
    }
    pool->status = RunningNormally;
    pthread_mutex_init(&pool->mutex,NULL); // initialize the struct's mutex
    pthread_cond_init(&pool->handleBusyWaiting,NULL); // initialize the cond
    for (i = 0; i<numOfThreads;i++){
        // create threads
        check = pthread_create(&pool->threads[i],NULL,executeTasks,pool);
        if (check != COMPLETE){
            free(pool->threads);
            osDestroyQueue(pool->tasksQueue);
            free(pool);
            handleFailure();
        }
    }
    return pool;
}

/**
 * This function is responsible of destroying the threadPool and
 * freeing all the allocated memory.
 * @param threadPool - pointer to the threadPool
 * @param shouldWaitForTasks - 0 -> should only wait for the running tasks
 * different from 0 -> should wait for the running tasks and to the
 * all tasks in the queue
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    pthread_mutex_lock(&threadPool->mutex);
    int status = threadPool->status; // lock critical section
    pthread_mutex_unlock(&threadPool->mutex);
    // If threadPool has been destroyed or uninitialized, return
    if (status != RunningNormally){
        return;
    }
    // should wait all the tasks (even to those which still in the queue)
    if (shouldWaitForTasks != 0){
        pthread_mutex_lock(&threadPool->mutex);
        // critical section
        threadPool->status = StopAndContinueExecuting; // change status
        pthread_cond_broadcast(&threadPool->handleBusyWaiting); // broadcast threads
        pthread_mutex_unlock(&threadPool->mutex);
    } else { // should wait only the running tasks
        pthread_mutex_lock(&threadPool->mutex);
        threadPool->status = Stopped; // change status
        pthread_cond_broadcast(&threadPool->handleBusyWaiting); // broadcast threads
        pthread_mutex_unlock(&threadPool->mutex);
    }
    int index;
    // wait for all the threads to end
    for (index = 0;index<threadPool->size;index++){
        pthread_join(threadPool->threads[index],NULL);
    }
    freeThreadPool(threadPool);
}

/**
 * This function is responsible of inserting a new task to handle
 * to the tasks queue
 * @param threadPool - pointer to thread pool
 * @param computeFunc - pointer to a funcion
 * @param param - param to the function
 * @return -1 if Insertion had failed and 0 otherwise
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    pthread_mutex_lock(&threadPool->mutex);
    int bool = threadPool->status; // lock critical section
    pthread_mutex_unlock(&threadPool->mutex);
    // If status is not RunningNormally than return -1
    if (bool != RunningNormally){
        return FAIL;
    }
    Func* newFunc = malloc(sizeof(Func)); // allocate data
    if (!newFunc){
        freeThreadPool(threadPool);
    }
    newFunc->computeFunc = computeFunc;
    newFunc->param = param;
    pthread_mutex_lock(&threadPool->mutex);
    osEnqueue(threadPool->tasksQueue,newFunc); // lock critical section
    // inform one thread to handle the new task
    pthread_cond_signal(&threadPool->handleBusyWaiting);
    pthread_mutex_unlock(&threadPool->mutex);
    return COMPLETE;
}

/**
 * This function handles the tasks by dequeuing it from the queue
 * and executing them.
 * If there is no tasks in the queue the thread stops at the "wait"
 * function until a signal received. In this way I solved busy waiting
 * and the loop is not running for nothing
 * @param threadPool - pointer to the threadpool struct
 */
void* executeTasks(void* threadPoolVoid) {
    ThreadPool* threadPool = threadPoolVoid;
    // while status is not stopped
    while (threadPool->status != Stopped){
        pthread_mutex_lock(&threadPool->mutex);
        // check status again
        if (threadPool->status != Stopped){
            // check queue
            if (!osIsQueueEmpty(threadPool->tasksQueue)){
                Func* func  = osDequeue(threadPool->tasksQueue);
                pthread_mutex_unlock(&threadPool->mutex);
                // executing task
                func->computeFunc(func->param);
                free(func);
            } else {
                // if status is StopAndContinueExecuting than return
                // and stop the thread
                if (threadPool->status == StopAndContinueExecuting){
                    pthread_mutex_unlock(&threadPool->mutex);
                    return NULL;
                }
                // if there is not a task in the queue than block the funcion
                // and wait for a signal to wake up
                pthread_cond_wait(&threadPool->handleBusyWaiting,&threadPool->mutex);
                pthread_mutex_unlock(&threadPool->mutex);
            }
        } else { // status is stopped
            pthread_mutex_unlock(&threadPool->mutex);
            return NULL;
        }
    }
}

/**
 * This function is being called in case of an error,
 * I wrote a msg for the user and exit the program
 */
void handleFailure(){
    write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
    exit(FAIL);
}

/**
 * This function frees all of the data in the ThreadPool and
 * frees the thread pool itself
 * @param threadPool
 */
void freeThreadPool(ThreadPool *threadPool){
    free(threadPool->threads); // free the dynamic array of threads
    // all thread finished so no need to use mutex
    while(!osIsQueueEmpty(threadPool->tasksQueue)){
        // empty the queue
        Func* data = osDequeue(threadPool->tasksQueue);
        free(data);
    }
    free(threadPool->tasksQueue); // free the queue
    // destroy mutex and cond
    pthread_mutex_destroy(&threadPool->mutex);
    pthread_cond_destroy(&threadPool->handleBusyWaiting);
    free(threadPool); // free the struct
}