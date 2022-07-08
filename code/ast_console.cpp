/*
Project: Asteroids
File: ast_console.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast_generated.h"

#define CONSOLE_OUTPUT_MAX_STRING 128
function void AddToDebugConsoleOutput(DebugState *debugState, PlatformAPI platform, char *string, v4f colour = V4F(1.0f))
{
    for (s8 stringIndex = ARRAY_COUNT(debugState->consoleOutput) - 1; stringIndex >= 0; --stringIndex)
    {
        if (stringIndex == ARRAY_COUNT(debugState->consoleOutput) - 1)
        {
            platform.MemFree(debugState->consoleOutput[stringIndex].string);
        }
        
        if (stringIndex > 0)
        {
            debugState->consoleOutput[stringIndex] = debugState->consoleOutput[stringIndex - 1];
        }
        else
        {
            debugState->consoleOutput[stringIndex].string = (char *)platform.MemAlloc(CONSOLE_OUTPUT_MAX_STRING);
            stbsp_sprintf(debugState->consoleOutput[stringIndex].string, "%s", string);
            debugState->consoleOutput[stringIndex].colour = colour;
        }
    }
}

function void DebugConsole(Game_Memory *memory, Game_RenderCommands *commands, Game_Input *input, Game_State *gameState, Camera camera)
{
    DebugState *debugState = (DebugState *)memory->storage[DEBUG_STORAGE_INDEX].ptr;
    if (debugState)
    {
        ASSERT(camera.orthographic);
        v2f max = V2F((f32)commands->width, (f32)commands->height);
        s32 layer = DEBUG_LAYER + 50;
        PushRect(commands, camera, V2F(), max, 1.0f, 0.0f, layer, V4F(0.05f, 0.0f, 0.1f, 0.2f));
        PushRect(commands, camera, V2F(0.0f, max.y - 30.0f), max, 1.0f, 0.0f, layer + 1, V4F(V3F(0.05f), 1.0f));
        
        b32 commandSubmitted = false;
        
        for (s32 keyIndex = 0; keyIndex < ARRAY_COUNT(input->keyboard.keys); ++keyIndex)
        {
            Game_Key *key = &input->keyboard.keys[keyIndex];
            if (InputNoRepeat(key->state))
            {
                u8 value = key->value;
                
                if (key == &input->keyboard.keyBackspace)
                {
                    if (debugState->consoleCommandCursor > 0)
                    {
                        debugState->consoleCommand[--debugState->consoleCommandCursor] = 0;
                    }
                }
                else if (key == &input->keyboard.keyEnter)
                {
                    commandSubmitted = true;
                    break;
                }
                else if (value != 0)
                {
                    if ((value >= 65 && value <= 90) && !input->keyboard.keyShift.state.endedFrameDown)
                    {
                        value += 32;
                    }
                    else if (value == ';' && input->keyboard.keyShift.state.endedFrameDown)
                    {
                        value = ':';
                    }
                    else if (value == '-' && input->keyboard.keyShift.state.endedFrameDown)
                    {
                        value = '_';
                    }
                    if (debugState->consoleCommandCursor < ARRAY_COUNT(debugState->consoleCommand))
                    {
                        debugState->consoleCommand[debugState->consoleCommandCursor++] = value;
                    }
                }
            }
        }
        //stbsp_sprintf(debugState->consoleCommand, "Test {} !@# 123");
        
        v3f textPos = V3F(10.0f, max.y - 25.0f, 1.0f);
        PushText(commands, camera, debugState->consoleCommand, "Debug", textPos, 1.0f, layer + 2, V4F(1.0f));
        
        v2f min = V2F(textPos.x + GetStringWidth(commands, "Debug", debugState->consoleCommand), max.y - 25.0f);
        PushRect(commands, camera, min, min + V2F(2.0f, 20.0f), 1.0f, 0.0f, layer + 2, V4F(1.0f));
        
        if (commandSubmitted)
        {
            char commandString[128];
            stbsp_sprintf(commandString, "%s", debugState->consoleCommand);
            for (u8 charIndex = 0; charIndex < ARRAY_COUNT(debugState->consoleCommand); ++charIndex)
            {
                if (debugState->consoleCommand[charIndex] == 0)
                {
                    break;
                }
                else
                {
                    debugState->consoleCommand[charIndex] = 0;
                }
            }
            debugState->consoleCommandCursor = 0;
            
            Lexer lexer = {};
            lexer.cursor = commandString;
            CommandType commandType = CommandType_Invalid;
            Token commandTkn = {};
            Token paramTkn = {};
            Token valueTkn = {};
            
            {
                commandTkn = GetToken(&lexer);
                
                switch (commandTkn.type)
                {
                    case TokenType_Identifier:
                    {
                        if (StringsAreSame(commandTkn.text, "introspect", 10))
                        {
                            paramTkn = GetToken(&lexer);
                            
                            if (TokenIsType(&lexer, TokenType_Colon) && paramTkn.type == TokenType_Identifier && StringsAreSame(paramTkn.text, "Entity", 6))
                            {
                                valueTkn = GetToken(&lexer);
                                if (valueTkn.type == TokenType_Integer)
                                {
                                    // TODO(bSalmon): Bug with negative indices
                                    commandType = CommandType_Introspect_Valid;
                                }
                                else
                                {
                                    commandType = CommandType_Introspect_InvalidIndex;
                                }
                            }
                            else
                            {
                                commandType = CommandType_Introspect_InvalidStruct;
                            }
                        }
                    } break;
                    
                    default: {} break;
                }
            }
            
            switch (commandType)
            {
                case CommandType_Invalid:
                {
                    char string[32] = {};
                    stbsp_sprintf(string, "Unknown Command: '%.*s'", commandTkn.length, commandTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                // TODO(bSalmon): This only works with entities, should work with more (particles, game state, etc)
                case CommandType_Introspect_Valid:
                {
                    s32 index = TokenToInt(valueTkn);
                    if (index < ARRAY_COUNT(gameState->entities) && index >= 0)
                    {
                        Entity *entity = &gameState->entities[index];
                        
                        Meta_DumpStruct(ARRAY_COUNT(introspected_Entity), introspected_Entity, entity, debugState, memory->platform, 0, 0);
                        
                        char string[32] = {};
                        stbsp_sprintf(string, "Found Entity at index %.*s", valueTkn.length, valueTkn.text);
                        AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(0.0f, 1.0f, 0.0f, 1.0f));
                    }
                    else
                    {
                        char string[32] = {};
                        stbsp_sprintf(string, "Invalid Entity Index %.*s", valueTkn.length, valueTkn.text);
                        AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                    
                    char attemptString[32] = {};
                    stbsp_sprintf(attemptString, "Attempting to introspect Entity at index %.*s", valueTkn.length, valueTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, attemptString, V4F(1.0f));
                } break;
                
                case CommandType_Introspect_InvalidStruct:
                {
                    char string[32] = {};
                    stbsp_sprintf(string, "Invalid introspect param: %.*s", paramTkn.length, paramTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                case CommandType_Introspect_InvalidIndex:
                {
                    char string[32] = {};
                    stbsp_sprintf(string, "Index must be an integer value: %.*s", valueTkn.length, valueTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                INVALID_DEFAULT;
            }
        }
        
        v3f outputPos = V3F(10.0f, max.y - 50.0f, 1.0f);
        for (u8 stringIndex = 0; stringIndex < ARRAY_COUNT(debugState->consoleOutput); ++stringIndex)
        {
            if (debugState->consoleOutput[stringIndex].string && StringLength(debugState->consoleOutput[stringIndex].string))
            {
                PushText(commands, camera, debugState->consoleOutput[stringIndex].string, "Debug", outputPos, 0.9f, layer + 2, debugState->consoleOutput[stringIndex].colour);
            }
            outputPos.y -= 20.0f;
        }
    }
}