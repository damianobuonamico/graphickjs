#include "Threads.h"
#ifndef GK_PLATFORM_WINDOWS
#include <unistd.h>
#endif


Threads::Threads()
{
}


Threads::~Threads()
{
}


int Threads::GetHardwareThreadCount()
{
#ifdef GK_PLATFORM_WINDOWS
    return std::thread::hardware_concurrency();
#else
    return Max(static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN)), 1);
#endif
}


void Threads::Run(const int count, Function* loopBody)
{
    ASSERT(loopBody != nullptr);

    if (count < 1) {
        return;
    }

    if (count == 1) {
        loopBody->Execute(0, mMainMemory);
        return;
    }

    mTaskData->Cursor = 0;
    mTaskData->Count = count;
    mTaskData->Fn = loopBody;

    const int threadCount = Min(mThreadCount, count);

    mTaskData->RequiredWorkerCount = threadCount;
    mTaskData->FinalizedWorkers = 0;

#ifdef GK_PLATFORM_WINDOWS
    mTaskData->FinalizationMutex.lock();

    // Wake all threads waiting on this condition variable.
    mTaskData->CV.notify_all();

    std::unique_lock<std::mutex> lock(mTaskData->FinalizationMutex);
    while (mTaskData->FinalizedWorkers < threadCount) {
        mTaskData->FinalizationCV.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::milliseconds(1));
    }

    mTaskData->FinalizationMutex.unlock();
#else
    pthread_mutex_lock(&mTaskData->FinalizationMutex);

    // Wake all threads waiting on this condition variable.
    pthread_cond_broadcast(&mTaskData->CV);

    while (mTaskData->FinalizedWorkers < threadCount) {
        pthread_cond_wait(&mTaskData->FinalizationCV,
            &mTaskData->FinalizationMutex);
    }

    pthread_mutex_unlock(&mTaskData->FinalizationMutex);
#endif

    // Cleanup.
    mTaskData->Cursor = 0;
    mTaskData->Count = 0;

    mTaskData->Fn = nullptr;

    mTaskData->RequiredWorkerCount = 0;
    mTaskData->FinalizedWorkers = 0;
}


void Threads::ResetFrameMemory()
{
    for (int i = 0; i < mThreadCount; i++) {
        mThreadData[i]->Memory.ResetFrameMemory();
    }

    mMainMemory.ResetFrameMemory();
}


void Threads::RunThreads()
{
    if (mThreadData != nullptr) {
        return;
    }

    mTaskData = new TaskList();

    const int cpuCount = Min(GetHardwareThreadCount(), 128);

    mThreadCount = cpuCount;

    mThreadData = new ThreadData * [cpuCount];

    for (int i = 0; i < cpuCount; i++) {
        mThreadData[i] = new ThreadData(mTaskData);
    }

    for (int i = 0; i < cpuCount; i++) {
        ThreadData* d = mThreadData[i];

#ifdef GK_PLATFORM_WINDOWS
        d->Thread = std::thread(Worker, d);
#else
        pthread_create(&d->Thread, nullptr, Worker, d);
#endif
    }
}


void* Threads::Worker(void* p)
{
    ASSERT(p != nullptr);

#ifndef __EMSCRIPTEN__
#ifndef GK_PLATFORM_WINDOWS
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif
#endif // __EMSCRIPTEN__

    ThreadData* d = reinterpret_cast<ThreadData*>(p);

    // Loop forever waiting for next dispatch of tasks.
    for (;;) {
        TaskList* items = d->Tasks;

#ifdef GK_PLATFORM_WINDOWS
        items->Mutex.lock();

        std::unique_lock<std::mutex> lock(items->Mutex);
        while (items->RequiredWorkerCount < 1) {
            // Wait until required worker count becomes greater than zero.
            items->CV.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::milliseconds(1));
        }

        items->Mutex.unlock();
#else
        pthread_mutex_lock(&items->Mutex);

        while (items->RequiredWorkerCount < 1) {
            // Wait until required worker count becomes greater than zero.
            pthread_cond_wait(&items->CV, &items->Mutex);
        }

        items->RequiredWorkerCount--;

        pthread_mutex_unlock(&items->Mutex);
#endif

        const int count = items->Count;

        for (;;) {
            const int index = items->Cursor++;

            if (index >= count) {
                break;
            }

            items->Fn->Execute(index, d->Memory);
        }

#ifdef GK_PLATFORM_WINDOWS
        items->FinalizationMutex.lock();

        items->FinalizedWorkers++;

        items->FinalizationMutex.unlock();

        items->FinalizationCV.notify_one();
#else
        pthread_mutex_lock(&items->FinalizationMutex);

        items->FinalizedWorkers++;

        pthread_mutex_unlock(&items->FinalizationMutex);

        pthread_cond_signal(&items->FinalizationCV);
#endif
    }
}
