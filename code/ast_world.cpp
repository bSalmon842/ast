/*
Project: Asteroids
File: ast_world.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

function World InitialiseWorld(v2f dims, WorldBorderType border)
{
    World result = {};
    
    result.area = CreateRect2f_MinDims(V2F(), dims);
    result.border = border;
    
    return result;
}

function void CheckWorldPosition(World world, Entity entity, v2f *pos)
{
    switch (world.border)
    {
        case WorldBorder_None: { } break;
        
        case WorldBorder_Loop:
        {
            if (pos->x + (entity.dims.w / 2.0f) < world.area.min.x)
            {
                pos->x = world.area.max.w + (entity.dims.w / 2.0f);
            }
            if (pos->x - (entity.dims.w / 2.0f) > world.area.max.x)
            {
                pos->x = world.area.min.w - (entity.dims.w / 2.0f);
            }
            if (pos->y + (entity.dims.h / 2.0f) < world.area.min.y)
            {
                pos->y = world.area.max.h + (entity.dims.h / 2.0f);
            }
            if (pos->y - (entity.dims.h / 2.0f) > world.area.max.y)
            {
                pos->y = world.area.min.h - (entity.dims.h / 2.0f);
            }
        } break;
        
        case WorldBorder_Stop:
        {
            *pos = ClampV2F(*pos, world.area.min, world.area.max);
        } break;
        
        INVALID_DEFAULT;
    };
}