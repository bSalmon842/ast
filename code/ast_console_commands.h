/*
Project: Asteroids
File: ast_console_commands.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_CONSOLE_COMMANDS_H

#define MAX_PARAM_COUNT 8
enum CommandLayout
{
    CommandLayout_String,
    CommandLayout_ID,
    CommandLayout_ID_V3F,
};

struct ConsoleCommandDesc
{
    char command[16];
    
    CommandLayout layout;
    u8 reqParamCount;
    
    // TODO(bSalmon): validStrings probably needs to hold more data in the future
    union
    {
        struct { f32 validMinF, validMaxF; };
        struct { s32 validMinI, validMaxI; };
        struct { v3f validMinV3F, validMaxV3F; };
    };
};

#define BaseCommandInfoFill \
stbsp_sprintf(result.command, "%s", command); \
result.layout = layout; \
result.reqParamCount = reqParamCount

inline ConsoleCommandDesc MakeConsoleCommand(char *command, CommandLayout layout, u8 reqParamCount)
{
    ConsoleCommandDesc result = {};
    
    BaseCommandInfoFill;
    
    return result;
}

inline ConsoleCommandDesc MakeConsoleCommand(char *command, CommandLayout layout, u8 reqParamCount, f32 min, f32 max)
{
    ConsoleCommandDesc result = {};
    
    BaseCommandInfoFill;
    result.validMinF = min;
    result.validMaxF = max;
    
    return result;
}

inline ConsoleCommandDesc MakeConsoleCommand(char *command, CommandLayout layout, u8 reqParamCount, v3f min, v3f max)
{
    ConsoleCommandDesc result = {};
    
    BaseCommandInfoFill;
    result.validMinV3F = min;
    result.validMaxV3F = max;
    
    return result;
}

global ConsoleCommandDesc consoleCommandList[] =
{
    MakeConsoleCommand("introspect", CommandLayout_ID, 1),
    MakeConsoleCommand("set_pos", CommandLayout_ID_V3F, 4, V3F(0.0f), V3F(100.0f)),
    
    MakeConsoleCommand("help", CommandLayout_String, 1),
};

function void ListCommandsForHelpToConsole(Game_Memory *memory)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        for (s32 i = ARRAY_COUNT(consoleCommandList) - 2; i >= 0; --i)
        {
            AddToDebugConsoleOutput(debugState, memory->platform, consoleCommandList[i].command, V4F(1.0f));
        }
    }
}

function ConsoleCommandDesc GetCommandDescFromCommand(char *command)
{
    ConsoleCommandDesc result = {};
    
    for (s32 i = 0; i < ARRAY_COUNT(consoleCommandList) - 1; ++i)
    {
        ConsoleCommandDesc desc = consoleCommandList[i];
        if (StringsAreSame(command, desc.command, StringLength(desc.command)))
        {
            result = desc;
            break;
        }
    }
    
    return result;
}

function void GetHelpForCommand(Game_Memory *memory, char *command)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        ConsoleCommandDesc desc = GetCommandDescFromCommand(command);
        
        char string[2048] = {};
        switch (desc.layout)
        {
            // TODO(bSalmon): Each command should probably have a description that says what it actually does
            case CommandLayout_String:
            {
                stbsp_sprintf(string, "%s [String]", desc.command);
            } break;
            
            case CommandLayout_ID:
            {
                stbsp_sprintf(string, "%s [UniversalID]", desc.command);
            } break;
            
            case CommandLayout_ID_V3F:
            {
                stbsp_sprintf(string, "%s [UniversalID] [f32 X: {%.04f - %.04f}] [f32 Y: {%.04f - %.04f}] [f32 Z: {%.04f - %.04f}]",
                              desc.command, desc.validMinV3F.x, desc.validMaxV3F.x, desc.validMinV3F.y, desc.validMaxV3F.y, desc.validMinV3F.z, desc.validMaxV3F.z);
            } break;
            
            INVALID_DEFAULT;
        }
        
        AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(0.0f, 1.0f, 1.0f, 1.0f));
    }
}

#define AST_CONSOLE_COMMANDS_H
#endif //AST_CONSOLE_COMMANDS_H
