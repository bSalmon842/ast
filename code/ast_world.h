/*
Project: Asteroids
File: ast_world.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_WORLD_H

enum WorldBorderType
{
    WorldBorder_None,
    WorldBorder_Loop,
    WorldBorder_Stop,
};

struct World
{
    Rect2f area;
    WorldBorderType border;
};

#define AST_WORLD_H
#endif //AST_WORLD_H
