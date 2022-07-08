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
#define BS842_MAKE_STRUCTS
#include "../bs842_vector.h"
#include "../ast_math.h"
#include "../ast_token.h"
#include "../ast_token.cpp"

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

function void IntrospectEnum(Lexer *lexer)
{
    Token nameTkn = GetToken(lexer);
    printf("IntrospectEnumDef introspected_%.*s[] =\n", nameTkn.length, nameTkn.text);
    printf("{\n");
    
    Token enumTypeTkn = {};
    if (TokenIsType(lexer, TokenType_Colon))
    {
        enumTypeTkn = GetToken(lexer);
        GetToken(lexer); // NOTE(bSalmon): Eat the open brace
    }
    else
    {
        enumTypeTkn.type = TokenType_Identifier;
        enumTypeTkn.text = "u32";
        enumTypeTkn.length = 3;
    };
    
    u64 enumValue = 0;
    for (;;)
    {
        Token enumNameTkn = GetToken(lexer);
        if (enumNameTkn.type == TokenType_CloseBrace)
        {
            break;
        }
        else
        {
            Token checkTkn = GetToken(lexer);
            if (checkTkn.type == TokenType_Equals)
            {
                Token valueTkn = GetToken(lexer);
                enumValue = strtoull(valueTkn.text, 0, 0);
                
                printf("{ EnumType_%.*s, \"%.*s\", %llX },\n",
                       enumTypeTkn.length, enumTypeTkn.text,
                       enumNameTkn.length, enumNameTkn.text,
                       enumValue++);
                
                if (TokenIsType(lexer, TokenType_CloseBrace))
                {
                    break;
                }
            }
            else if (checkTkn.type == TokenType_Comma)
            {
                printf("{ EnumType_%.*s, \"%.*s\", 0x%016llX },\n",
                       enumTypeTkn.length, enumTypeTkn.text,
                       enumNameTkn.length, enumNameTkn.text,
                       enumValue++);
            }
        }
    }
    
    printf("};\n\n");
    
    printf("IntrospectEnumDef GetEnumDef_%.*s(u64 value)\n", nameTkn.length, nameTkn.text);
    printf("{\n");
    printf("IntrospectEnumDef result = {};\n\n");
    printf("for (u64 i = 0; i < ARRAY_COUNT(introspected_%.*s); ++i)\n", nameTkn.length, nameTkn.text);
    printf("{\n");
    printf("if (introspected_%.*s[i].value == value)\n", nameTkn.length, nameTkn.text);
    printf("{\n");
    printf("result = introspected_%.*s[i];\n", nameTkn.length, nameTkn.text);
    printf("break;\n");
    printf("}\n");
    printf("}\n\n");
    printf("return result;\n");
    printf("}\n\n");
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
                            IntrospectEnum(&lexer);
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
    
    printf("#ifndef CMT_GENERATED_H\n");
    printf("#define Preproc_MemberOffset(structName, member) (u32)&((structName *)0)->member\n\n");
    
    ParseFile("ast_entity.h");
    ParseFile("ast_collision.h");
    
    printf("#define GetEnumDef(type, value) GetEnumDef_##type##(value)\n\n");
    printf("#define CMT_GENERATED_H\n");
    printf("#endif // CMT_GENERATED_H\n");
    
    return 0;
}