/*
Project: Asteroids
File: ast_timer.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

inline Timer InitialiseTimer(f32 startTime, f32 target, b32 startPaused = false)
{
    Timer result = {};
    
    result.startTime = startTime;
    result.currTime = startTime;
    result.target = target;
    result.paused = startPaused;
    result.finished = false;
    
    return result;
}

// TODO(bSalmon): This seems like it might suck and there's surely a better way to do it
inline void UpdateTimer(Timer *timer, f32 deltaTime)
{
    if (!timer->finished && !timer->paused)
    {
        if (timer->currTime > timer->target)
        {
            // NOTE(bSalmon): Countdown
            timer->currTime -= deltaTime;
            if (timer->currTime <= timer->target)
            {
                timer->finished = true;
            }
        }
        else
        {
            // NOTE(bSalmon): Countup
            timer->currTime += deltaTime;
            if (timer->currTime >= timer->target)
            {
                timer->finished = true;
            }
        }
    }
}

inline void ToggleTimer(Timer *timer)
{
    timer->paused = !timer->paused;
}

inline void ResetTimer(Timer *timer)
{
    timer->currTime = timer->startTime;
    timer->finished = false;
}
