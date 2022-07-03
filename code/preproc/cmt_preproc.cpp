/*
Project: Asteroids
File: cmt_preproc.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "../ast_utility.h"

// TODO(bSalmon): Enum processing

function char *ReadFileContentsWithTerminator(char *filename)
{
    char *result = 0;
    
    FILE *file = fopen(filename, "r");
    if (file)
    {
        usize fileSize = 0;
        
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result = (char *)calloc(fileSize + 1, 1);
        
        fread(result, 1, fileSize, file);
        result[fileSize] = '\0';
        
        fclose(file);
    }
    
    return result;
}

enum TokenType
{
    TokenType_Unknown,
    
    TokenType_Identifier,
    TokenType_OpenParenthesis,
    TokenType_CloseParenthesis,
    TokenType_OpenBrace,
    TokenType_CloseBrace,
    TokenType_OpenBracket,
    TokenType_CloseBracket,
    TokenType_Colon,
    TokenType_Semicolon,
    TokenType_Asterisk,
    TokenType_Ampersand,
    TokenType_Comma,
    TokenType_Macro,
    TokenType_String,
    
    TokenType_EOS,
};

struct Token
{
    TokenType type;
    u32 length;
    char *text;
};

struct Lexer
{
    char *cursor;
};

inline b32 IsNewLine(char c)
{
    b32 result = (c == '\n' || c == '\r');
    return result;
}

inline b32 IsAlpha(char c)
{
    b32 result = ((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z'));
    return result;
}

inline b32 IsNumeric(char c)
{
    b32 result = (c >= '0' && c <= '9');
    return result;
}

inline void SkipWhitespace(Lexer *lexer)
{
    for (;;)
    {
        char *start = lexer->cursor;
        
        while (*lexer->cursor == ' ' ||
               *lexer->cursor == '\t' ||
               IsNewLine(*lexer->cursor))
        {
            ++lexer->cursor;
        }
        
        if (StringsAreSame(lexer->cursor, "//", 2))
        {
            while (*lexer->cursor && !IsNewLine(*lexer->cursor))
            {
                ++lexer->cursor;
            }
        }
        
        if (StringsAreSame(lexer->cursor, "/*", 2))
        {
            while (*lexer->cursor && !StringsAreSame(lexer->cursor, "*/", 2))
            {
                ++lexer->cursor;
            }
            
            lexer->cursor += 2;
        }
        
        if (StringsAreSame(lexer->cursor, "#if 0", 5))
        {
            while (*lexer->cursor && !StringsAreSame(lexer->cursor, "#endif", 6))
            {
                ++lexer->cursor;
            }
            
            lexer->cursor += 6;
        }
        
        if (lexer->cursor == start)
        {
            break;
        }
    }
}

function Token GetToken(Lexer *lexer)
{
    Token result = {};
    
    SkipWhitespace(lexer);
    
    result.length = 1;
    result.text = lexer->cursor;
    switch (*lexer->cursor)
    {
        case '(': { result.type = TokenType_OpenParenthesis; ++lexer->cursor; } break;
        case ')': { result.type = TokenType_CloseParenthesis; ++lexer->cursor; } break;
        case '{': { result.type = TokenType_OpenBrace; ++lexer->cursor; } break;
        case '}': { result.type = TokenType_CloseBrace; ++lexer->cursor; } break;
        case '[': { result.type = TokenType_OpenBracket; ++lexer->cursor; } break;
        case ']': { result.type = TokenType_CloseBracket; ++lexer->cursor; } break;
        case ':': { result.type = TokenType_Colon; ++lexer->cursor; } break;
        case ';': { result.type = TokenType_Semicolon; ++lexer->cursor; } break;
        case '*': { result.type = TokenType_Asterisk; ++lexer->cursor; } break;
        case '&': { result.type = TokenType_Ampersand; ++lexer->cursor; } break;
        case ',': { result.type = TokenType_Comma; ++lexer->cursor; } break;
        case '\0': { result.type = TokenType_EOS; ++lexer->cursor; } break;
        
        case '#':
        {
            ++lexer->cursor;
            
            result.type = TokenType_Macro;
            while (*lexer->cursor &&
                   !IsNewLine(*lexer->cursor))
            {
                ++lexer->cursor;
            }
            
            result.length = (u32)(lexer->cursor - result.text);
        } break;
        
        case '"':
        {
            ++lexer->cursor;
            
            result.type = TokenType_String;
            result.text = lexer->cursor;
            while (*lexer->cursor &&
                   *lexer->cursor != '"')
            {
                if (*lexer->cursor == '\\' && lexer->cursor[1])
                {
                    ++lexer->cursor;
                }
                
                ++lexer->cursor;
            }
            
            result.length = (u32)(lexer->cursor - result.text);
            ++lexer->cursor;
        } break;
        
        default:
        {
            result.type = TokenType_Identifier;
            if (IsAlpha(*lexer->cursor))
            {
                while (IsAlpha(*lexer->cursor) || IsNumeric(*lexer->cursor) || *lexer->cursor == '_')
                {
                    ++lexer->cursor;
                }
                
                result.length = (u32)(lexer->cursor - result.text);
            }
#if 0
            else if (IsNumeric(*lexer->cursor))
            {
                
            }
#endif
            else
            {
                result.type = TokenType_Unknown;
                ++lexer->cursor;
            }
        } break;
    }
    
    return result;
}

inline b32 TokenIsType(Lexer *lexer, TokenType type)
{
    Token tkn = GetToken(lexer);
    b32 result = (tkn.type == type);
    return result;
}

function void IntrospectStruct(Lexer *lexer)
{
    Token structTkn = GetToken(lexer);
    printf("IntrospectMemberDef introspected_%.*s[] =\n", structTkn.length, structTkn.text);
    printf("{\n");
    if (TokenIsType(lexer, TokenType_OpenBrace))
    {
        for (;;)
        {
            Token membertypeTkn = GetToken(lexer);
            if (membertypeTkn.type == TokenType_CloseBrace)
            {
                break;
            }
            else
            {
                // NOTE(bSalmon): Parse struct member
                b32 parsingMember = true;
                while (parsingMember)
                {
                    Token memberNameTkn = GetToken(lexer);
                    
                    switch(memberNameTkn.type)
                    {
                        case TokenType_Identifier:
                        {
                            Token memberArrayCheck = GetToken(lexer);
                            
                            if (memberArrayCheck.type == TokenType_OpenBracket)
                            {
                                Token arrayCountTkn = GetToken(lexer);
                                printf("{ MemberType_Array_%.*s, \"%.*s\", Preproc_MemberOffset(%.*s, %.*s), %.*s },\n",
                                       membertypeTkn.length, membertypeTkn.text,
                                       memberNameTkn.length, memberNameTkn.text,
                                       structTkn.length, structTkn.text,
                                       memberNameTkn.length, memberNameTkn.text,
                                       arrayCountTkn.length, arrayCountTkn.text);
                            }
                            else
                            {
                                printf("{ MemberType_%.*s, \"%.*s\", Preproc_MemberOffset(%.*s, %.*s), 1 },\n",
                                       membertypeTkn.length, membertypeTkn.text,
                                       memberNameTkn.length, memberNameTkn.text,
                                       structTkn.length, structTkn.text,
                                       memberNameTkn.length, memberNameTkn.text);
                                parsingMember = false;
                            }
                        } break;
                        
                        case TokenType_Semicolon:
                        case TokenType_EOS:
                        {
                            parsingMember = false;
                        } break;
                        
                        default: {} break;
                    }
                }
            }
        }
    }
    printf("};\n\n");
}

function void ParseFile(char *filename)
{
    char *fileContents = ReadFileContentsWithTerminator(filename);
    
    Lexer lexer = {};
    lexer.cursor = fileContents;
    
    for (;;)
    {
        Token tkn = GetToken(&lexer);
        if (tkn.type == TokenType_EOS)
        {
            break;
        }
        
        switch (tkn.type)
        {
            case TokenType_EOS:
            {
                ASSERT(false);
            } break;
            
#if 0            
            case TokenType_Unknown:
            {
                printf("Found Unknown Token: '%c'\n", *tkn.text);
            } break;
#endif
            
            case TokenType_Identifier:
            {
                if (StringsAreSame(tkn.text, "introspect", 10))
                {
                    if (TokenIsType(&lexer, TokenType_OpenParenthesis))
                    {
                        // NOTE(bSalmon): Parse introspect params
                        Token paramTkn = GetToken(&lexer);
                        while (paramTkn.type != TokenType_CloseParenthesis && paramTkn.type != TokenType_EOS)
                        {
                            paramTkn = GetToken(&lexer);
                        }
                        
                        // NOTE(bSalmon): Parse struct
                        Token intrTkn = GetToken(&lexer);
                        if (StringsAreSame(intrTkn.text, "struct", 6))
                        {
                            IntrospectStruct(&lexer);
                        }
                        else if (StringsAreSame(intrTkn.text, "enum", 4))
                        {
                            
                        }
                        else
                        {
                            printf("ERROR: NON-STRUCT INTROSPECTION");
                        }
                    }
                    else
                    {
                        printf("ERROR: NO INTROSPECT PARAMETERS");
                    }
                }
            } break;
            
            default: {} break;
        }
    }
}

s32 main(s32 args, char **argv)
{
    _chdir("..\\..\\code");
    
    printf("#define Preproc_MemberOffset(structName, member) (u32)&((structName *)0)->member\n\n");
    
    ParseFile("ast_entity.h");
    ParseFile("ast_collision.h");
    
    return 0;
}