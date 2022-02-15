/*
Project: Asteroids
File: ast_entity.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_ENTITY_H

enum AsteroidSize
{
    AsteroidSize_Large,
    AsteroidSize_Medium,
    AsteroidSize_Small,
    
    AsteroidSize_Count,
};

enum Entity_Type
{
    Entity_Null,
    
    Entity_Player,
    Entity_Shot,
    Entity_Asteroids,
    
    Entity_Count,
};

typedef struct
{
    Entity_Type type;
    
    b32 active;
    f32 angle;
    v2f pos;
    v2f dP;
    v2f dims;
} Entity;

#define AST_ENTITY_H
#endif //AST_ENTITY_H
