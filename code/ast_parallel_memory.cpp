/*
Project: Asteroids
File: ast_parallel_memory.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

inline ParallelMemory *StartParallelMemory(Transient_State *transState)
{
    ParallelMemory *result = 0;
    
    for (u32 i = 0; i < ARRAY_COUNT(transState->parallelMems); ++i)
    {
        ParallelMemory *mem = &transState->parallelMems[i];
        if (!mem->inUse)
        {
            result = mem;
            mem->memFlush = StartTempMemory(&mem->memRegion);
            mem->inUse = true;
            break;
        }
    }
    
    return result;
}

inline void FinishParallelMemory(ParallelMemory *mem)
{
    FinishTempMemory(mem->memFlush);
    
    WriteBarrier;
    
    mem->inUse = false;
}
