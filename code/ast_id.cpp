/*
Project: Asteroids
File: ast_id.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

global UniversalIDList *globalIDList;

inline u16 FindFirstAvailableNumberForIdentifier(char *identifier)
{
    u16 result = 10000;
    
    char searchString[64] = {};
    stbsp_sprintf(searchString, "%s_", identifier);
    
    for (u16 searchNumber = 0; searchNumber < 10000; ++searchNumber)
    {
        b32 numberFound = false;
        // TODO(bSalmon): Store where the last used index is in UniversalIDList to optimise this function
        for (u32 idIndex = 0; idIndex < MAX_ID_COUNT; ++idIndex)
        {
            UniversalID *id = &globalIDList->ids[idIndex];
            if (StringsAreSame(id->string, searchString, StringLength(searchString)))
            {
                if (id->number == searchNumber)
                {
                    numberFound = true;
                    break;
                }
            }
        }
        
        if (!numberFound)
        {
            result = searchNumber;
            break;
        }
    }
    
    return result;
}

function UniversalID *RegisterUniversalID(char *identifier, IDPtrType type, void *ptr)
{
    UniversalID *result = 0;
    
    u16 number = FindFirstAvailableNumberForIdentifier(identifier);
    if (number < 10000 && globalIDList->firstAvailIndex < U16_MAX)
    {
        UniversalID *id = &globalIDList->ids[globalIDList->firstAvailIndex];
        stbsp_sprintf(id->string, "%s_%04d", identifier, number);
        id->number = number;
        id->type = type;
        id->ptr = ptr;
        
        result = id;
        
        for (s32 idIndex = globalIDList->firstAvailIndex; idIndex < MAX_ID_COUNT; ++idIndex)
        {
            UniversalID availID = globalIDList->ids[idIndex];
            if (availID.type == IDPtrType_Null)
            {
                globalIDList->firstAvailIndex = idIndex;
                break;
            }
        }
    }
    
    return result;
}

function void DeregisterUniversalID(char *idString)
{
    if (idString)
    {
        // TODO(bSalmon): Store where the last used index is in UniversalIDList to optimise this function
        for (u32 idIndex = 0; idIndex < MAX_ID_COUNT; ++idIndex)
        {
            UniversalID *id = &globalIDList->ids[idIndex];
            if (StringsAreSame(id->string, idString, StringLength(idString)))
            {
                if (idIndex < globalIDList->firstAvailIndex)
                {
                    globalIDList->firstAvailIndex = (u16)idIndex;
                }
                
                stbsp_sprintf(id->string, "\0");
                id->number = 0;
                id->type = IDPtrType_Null;
                id->ptr = 0;
            }
        }
    }
}

function UniversalID GetIDInfo(char *string, u32 stringLength)
{
    UniversalID result = {};
    
    // TODO(bSalmon): Store where the last used index is in UniversalIDList to optimise this function
    for (u32 idIndex = 0; idIndex < MAX_ID_COUNT; ++idIndex)
    {
        UniversalID id = globalIDList->ids[idIndex];
        if (StringsAreSame(id.string, string, stringLength))
        {
            result = id;
            break;
        }
    }
    
    return result;
}

function b32 IDExists(char *string, u32 stringLength)
{
    b32 result = false;
    
    // TODO(bSalmon): Store where the last used index is in UniversalIDList to optimise this function
    for (u32 idIndex = 0; idIndex < MAX_ID_COUNT; ++idIndex)
    {
        UniversalID *id = &globalIDList->ids[idIndex];
        if (StringsAreSame(id->string, string, stringLength))
        {
            result = true;
            break;
        }
    }
    
    return result;
}