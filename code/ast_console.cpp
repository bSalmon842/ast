/*
Project: Asteroids
File: ast_console.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include "ast_generated.h"
#include "ast_console_commands.h"

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
            CommandType commandType = CommandType_Valid;
            Token commandTkn = {};
            Token paramTkns[MAX_PARAM_COUNT] = {};
            Token invalidParamTkn = {};
            s32 paramCount = 0;
            ConsoleCommandDesc commandDesc = {};
            b32 isHelpCommand = false;
            
            {
                commandTkn = GetToken(&lexer);
                
                switch (commandTkn.type)
                {
                    case TokenType_Identifier:
                    {
                        s32 foundCommandIndex = -1;
                        for (s32 commandIndex = 0; commandIndex < ARRAY_COUNT(consoleCommandList); ++commandIndex)
                        {
                            char *command = consoleCommandList[commandIndex].command;
                            if (StringsAreSame(commandTkn.text, command, StringLength(command)))
                            {
                                foundCommandIndex = commandIndex;
                            }
                        }
                        
                        if (foundCommandIndex > -1)
                        {
                            for (;;)
                            {
                                Token tkn = GetToken(&lexer);
                                if (tkn.type == TokenType_EOS)
                                {
                                    break;
                                }
                                else if (tkn.type == TokenType_Identifier ||
                                         tkn.type == TokenType_String ||
                                         tkn.type == TokenType_Integer ||
                                         tkn.type == TokenType_Float)
                                {
                                    paramTkns[paramCount++] = tkn;
                                }
                            }
                            
                            isHelpCommand = StringsAreSame("help", commandTkn.text, commandTkn.length);
                            commandDesc = consoleCommandList[foundCommandIndex];
                            if (paramCount < commandDesc.reqParamCount && !isHelpCommand)
                            {
                                commandType = CommandType_Invalid_ParamCount;
                            }
                            else
                            {
                                CommandLayout layout = commandDesc.layout;
                                switch (layout)
                                {
                                    case CommandLayout_String: {} break;
                                    
                                    case CommandLayout_ID:
                                    {
                                        if (!IDExists(paramTkns[0].text, paramTkns[0].length))
                                        {
                                            commandType = CommandType_Invalid_Param;
                                            invalidParamTkn = paramTkns[0];
                                        }
                                    } break;
                                    
                                    case CommandLayout_ID_V3F:
                                    {
                                        if (!IDExists(paramTkns[0].text, paramTkns[0].length))
                                        {
                                            commandType = CommandType_Invalid_Param;
                                            invalidParamTkn = paramTkns[0];
                                        }
                                        
                                        f32 x = TokenToFloat(paramTkns[1]);
                                        if (x < commandDesc.validMinV3F.x || x > commandDesc.validMaxV3F.x)
                                        {
                                            commandType = CommandType_Invalid_Param;
                                            invalidParamTkn = paramTkns[1];
                                            break;
                                        }
                                        
                                        f32 y = TokenToFloat(paramTkns[2]);
                                        if (y < commandDesc.validMinV3F.y || y > commandDesc.validMaxV3F.y)
                                        {
                                            commandType = CommandType_Invalid_Param;
                                            invalidParamTkn = paramTkns[2];
                                            break;
                                        }
                                        
                                        f32 z = TokenToFloat(paramTkns[3]);
                                        if (z < commandDesc.validMinV3F.z || z > commandDesc.validMaxV3F.z)
                                        {
                                            commandType = CommandType_Invalid_Param;
                                            invalidParamTkn = paramTkns[3];
                                            break;
                                        }
                                    } break;
                                    
                                    INVALID_DEFAULT;
                                }
                            }
                        }
                        else
                        {
                            commandType = CommandType_Invalid_Command;
                        }
                    } break;
                    
                    default: {} break;
                }
            }
            
            switch (commandType)
            {
                case CommandType_Invalid_Command:
                {
                    char string[64] = {};
                    stbsp_sprintf(string, "Unknown Command: '%.*s'", commandTkn.length, commandTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                case CommandType_Invalid_ParamCount:
                {
                    char string[64] = {};
                    stbsp_sprintf(string, "Too Few Parameters: Expected %d, Got %d", commandDesc.reqParamCount, paramCount);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                case CommandType_Invalid_Param:
                {
                    char string[64] = {};
                    stbsp_sprintf(string, "Invalid Parameter: %.*s", invalidParamTkn.length, invalidParamTkn.text);
                    AddToDebugConsoleOutput(debugState, memory->platform, string, V4F(1.0f, 0.0f, 0.0f, 1.0f));
                } break;
                
                case CommandType_Valid:
                {
                    CommandLayout layout = commandDesc.layout;
                    switch (layout)
                    {
                        case CommandLayout_String:
                        {
                            if (isHelpCommand)
                            {
                                if (paramCount > 0)
                                {
                                    GetHelpForCommand(memory, paramTkns[0].text);
                                }
                                else
                                {
                                    ListCommandsForHelpToConsole(memory);
                                }
                            }
                            else
                            {
                                
                            }
                        } break;
                        
                        case CommandLayout_ID:
                        {
                            UniversalID id = GetIDInfo(paramTkns[0].text, paramTkns[0].length);
                            if (StringsAreSame("introspect", commandTkn.text, commandTkn.length))
                            {
                                switch (id.type)
                                {
                                    case IDPtrType_Entity:
                                    {
                                        Entity *entity = (Entity *)id.ptr;
                                        Meta_DumpStruct(ARRAY_COUNT(introspected_Entity), introspected_Entity, entity, debugState, memory->platform, 0, 0);
                                    } break;
                                    
                                    INVALID_DEFAULT;
                                }
                            }
                        } break;
                        
                        case CommandLayout_ID_V3F:
                        {
                            UniversalID id = GetIDInfo(paramTkns[0].text, paramTkns[0].length);
                            if (StringsAreSame("set_pos", commandTkn.text, commandTkn.length))
                            {
                                switch (id.type)
                                {
                                    case IDPtrType_Entity:
                                    {
                                        Entity *entity = (Entity *)id.ptr;
                                        entity->pos = V3F(TokenToFloat(paramTkns[1]), TokenToFloat(paramTkns[2]), TokenToFloat(paramTkns[3]));
                                    } break;
                                    
                                    INVALID_DEFAULT;
                                }
                            }
                        } break;
                        
                        INVALID_DEFAULT;
                    }
                } break;
                
                INVALID_DEFAULT;
            }
        }
        
        v3f outputPos = V3F(10.0f, max.y - 50.0f, 1.0f);
        for (u8 stringIndex = 0; stringIndex < ARRAY_COUNT(debugState->consoleOutput); ++stringIndex)
        {
            if (debugState->consoleOutput[stringIndex].string && StringLength(debugState->consoleOutput[stringIndex].string))
            {
                PushText(commands, camera, debugState->consoleOutput[stringIndex].string, "Debug", outputPos, 0.75f, layer + 2, debugState->consoleOutput[stringIndex].colour);
            }
            outputPos.y -= 15.0f;
        }
    }
}