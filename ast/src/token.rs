use std::fmt::{self, Debug, Display, Formatter};

/// File: token.rs
/// Author: Devin Arena
/// Date: 12/2/2022
/// Description: Tokens store information for lexed text

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum TokenType {
    PLUS,
    MINUS,
    STAR,
    SLASH,
    SEMICOLON,
    LEFT_PAREN,
    RIGHT_PAREN,

    LITERAL_INTEGER,
    LITERAL_FLOAT,
    LITERAL_STRING,

    ERROR,
    EOF,
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
            line,
        }
    }
}
