use std::fmt::Debug;


/// File: token.rs
/// Author: Devin Arena
/// Date: 12/2/2022
/// Description: Tokens store information for lexed text

#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    INTEGER,
    FLOAT,
    PLUS,
    MINUS,
    SEMICOLON,
    EOF
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String
}

impl Token {
    pub fn new(token_type: TokenType, lexeme: String) -> Token {
        Token {
            token_type,
            lexeme
        }
    }
}