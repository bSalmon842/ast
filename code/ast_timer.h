/*
Project: Asteroids
File: ast_timer.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_TIMER_H

struct Timer
{
    f32 startTime;
    f32 currTime;
    f32 target;
    b32 paused;
    b32 finished;
};

#define AST_TIMER_H
#endif //AST_TIMER_H
