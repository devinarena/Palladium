use std::fmt::Debug;


/// File: token.rs
/// Author: Devin Arena
/// Date: 12/2/2022
/// Description: Tokens store information for lexed text

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum TokenType {
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,

    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,

    LEFT_PAREN,
    RIGHT_PAREN,
    SEMICOLON,

    IDENTIFIER,
    PRINT,
    LET,

    EOF
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    pub line: usize
}

impl Token {
    pub fn new(token_type: TokenType, lexeme: String, line: usize) -> Token {
        Token {
            token_type,
            lexeme,
            line
        }
    }
}