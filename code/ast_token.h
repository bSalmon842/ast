/*
Project: Asteroids
File: ast_token.h
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

#ifndef AST_TOKEN_H

enum TokenType
{
    TokenType_Unknown,
    
    TokenType_Identifier,
    TokenType_Integer,
    TokenType_Float,
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
    TokenType_Equals,
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

#define AST_TOKEN_H
#endif //AST_TOKEN_H
