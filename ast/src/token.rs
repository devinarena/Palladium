use std::fmt::{Debug, Display, Formatter, self};


/// File: token.rs
/// Author: Devin Arena
/// Date: 12/2/2022
/// Description: Tokens store information for lexed text

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum TokenType {
    INTEGER,
    FLOAT,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    SEMICOLON,
    ERROR,
    EOF
}

impl Display for TokenType {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    pub line: usize,
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