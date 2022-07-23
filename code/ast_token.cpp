/*
Project: Asteroids
File: ast_token.cpp
Author: Brock Salmon
Notice: (C) Copyright 2022 by Brock Salmon. All Rights Reserved
*/

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
        case '=': { result.type = TokenType_Equals; ++lexer->cursor; } break;
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
            else if (IsNumeric(*lexer->cursor) || *lexer->cursor == '-')
            {
                result.type = TokenType_Integer;
                ++lexer->cursor;
                while (IsNumeric(*lexer->cursor) || *lexer->cursor == '.')
                {
                    if (*lexer->cursor == '.')
                    {
                        result.type = TokenType_Float;
                    }
                    ++lexer->cursor;
                }
                
                result.length = (u32)(lexer->cursor - result.text);
            }
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

inline s32 TokenToInt(Token tkn)
{
    s32 result = 0;
    
    b32 negative = (tkn.text[0] == '-');
    s32 lengthRemaining = (negative) ? tkn.length - 1 : tkn.length;
    tkn.text = (negative) ? &tkn.text[1] : tkn.text;
    
    if (lengthRemaining > 1)
    {
        for (s32 i = 2; i <= lengthRemaining; ++i)
        {
            s32 index = lengthRemaining - i;
            s32 power = index + 1;
            result += CharToInt(tkn.text[index]) * RoundF32ToS32(Pow(10, (f32)power));
        }
    }
    
    result += CharToInt(tkn.text[lengthRemaining - 1]);
    result *= (negative) ? -1 : 1;
    
    return result;
}

inline f32 TokenToFloat(Token tkn)
{
    f32 result = 0;
    
    b32 negative = (tkn.text[0] == '-');
    
    s32 decimalPos = -1;
    for (u32 i = 0; i < tkn.length; ++i)
    {
        if (tkn.text[i] == '.')
        {
            decimalPos = i;
            break;
        }
    }
    
    Token exponent = {};
    exponent.text = (negative) ? &tkn.text[1] : tkn.text;
    exponent.length = ((u32)&tkn.text[decimalPos] - (u32)&exponent.text[0]);
    
    Token mantissa = {};
    if (decimalPos != -1)
    {
        mantissa.text = &tkn.text[decimalPos + 1];
        mantissa.length = tkn.length - (decimalPos + 1);
    }
    
    result += (f32)TokenToInt(exponent);
    
    if (mantissa.length > 0)
    {
        f32 decAdjust = Pow(10, (f32)mantissa.length);
        result += ((f32)TokenToInt(mantissa) / decAdjust);
    }
    
    result *= (negative) ? -1 : 1;
    
    return result;
}