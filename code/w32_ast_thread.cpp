/*
Project: Asteroids
File: w32_thread.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

/*
MULTITHREADING NOTES
- Write and Read Barriers are used to stop the compiler moving writes or reads that are placed after the barrier to somewhere
  before the barrier

- InterlockedIncrement(&a) acts as ++a rather than a++
- InterlockedCompareExchange(a, b, c), when a == c, a = b, meaning only one thread gets to change it and proceed with the old value

- Interlocked functions act as processor barriers, which means putting barriers after them _MAY_ not be totally necessary

- Semaphores are used to alert the OS when we are done with a thread, and when we need to use the thread again, the OS will put
the thread into a low-power state when not being used
*/

#define WriteBarrier _WriteBarrier()
#define ReadBarrier _ReadBarrier()
#define ReadWriteBarrier _ReadWriteBarrier()

struct W32_ParallelSemaphore
{
    HANDLE semaphore;
};

struct W32_Thread
{
    s32 threadIndex;
    Platform_ParallelQueue *queue;
};

function b32 ProcessNextParallelQueueEntry(Platform_ParallelQueue *queue)
{
    b32 result = false; // Whether the thread should sleep
    
    u32 nextEntryToReadOriginal = queue->nextEntryToRead;
    u32 newReadEntry = (nextEntryToReadOriginal + 1) % ARRAY_COUNT(queue->entries);
    if (queue->nextEntryToRead != queue->nextEntryToWrite)
    {
        u32 index = AtomicCompareExchange(&queue->nextEntryToRead, newReadEntry, nextEntryToReadOriginal);
        if (index == nextEntryToReadOriginal)
        {
            ParallelWorkEntry entry = queue->entries[index];
            entry.callback(entry.data);
            
            AtomicIncrement(&queue->completedCount);
        }
    }
    else
    {
        result = true;
    }
    
    return result;
}

DWORD WINAPI ThreadProc(LPVOID param)
{
    W32_Thread *thread = (W32_Thread *)param;
    
    for (;;)
    {
        if (ProcessNextParallelQueueEntry(thread->queue))
        {
            WaitForSingleObjectEx(thread->queue->semaphore, INFINITE, FALSE);
        }
    }
}

function PLATFORM_ADD_PARALLEL_ENTRY(W32_AddParallelEntry)
{
    u32 newWriteEntry = (queue->nextEntryToWrite + 1) % ARRAY_COUNT(queue->entries);
    ASSERT(newWriteEntry != queue->nextEntryToRead);
    ParallelWorkEntry *entry = &queue->entries[queue->nextEntryToWrite];
    entry->callback = callback;
    entry->data = data;
    ++queue->completionTarget;
    
    WriteBarrier;
    queue->nextEntryToWrite = newWriteEntry;
    
    ReleaseSemaphore(queue->semaphore, 1, 0);
}

function PLATFORM_COMPLETE_ALL_PARALLEL_WORK(W32_CompleteAllParallelWork)
{
    while (queue->completedCount != queue->completionTarget)
    {
        ProcessNextParallelQueueEntry(queue);
    }
    
    queue->completionTarget = 0;
    queue->completedCount = 0;
}

function PLATFORM_HAS_PARALLEL_WORK_FINISHED(W32_HasParallelWorkFinished)
{
    b32 result = queue->completedCount != queue->completionTarget;
    return result;
}
