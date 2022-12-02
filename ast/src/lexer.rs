use std::{fs, fmt::Debug};

use crate::token::{Token, TokenType};


/// File: lexer.rs
/// Author: Devin Arena
/// Date: 12/1/2022
/// Description: Lexer for the AST
#[derive(Debug, Clone)]
pub struct Lexer {
    input: String,
    position: usize,
    line: usize,
}

impl Lexer {
    fn read_source(input: String) -> String {
        let contents = fs::read_to_string(input).expect("Unable to read file");
        contents
    }

    pub fn new(input: String) -> Lexer {
        Lexer {
            input: Self::read_source(input),
            position: 0,
            line: 0,
        }
    }

    fn peek(&self) -> char {
        if self.position >= self.input.len() {
            return '\0';
        }

        self.input.chars().nth(self.position).unwrap()
    }

    fn peek_next(&mut self) -> char {
        if self.position + 1 >= self.input.len() {
            return '\0';
        }

        self.input.chars().nth(self.position + 1).unwrap()
    }

    fn advance(&mut self) {
        self.position += 1;
    }

    fn skip_whitespace(&mut self) {
        while self.peek().is_whitespace() {
            if self.peek() == '\0' {
                break;
            }

            if self.peek() == '\n' {
                self.line += 1;
            }

            self.advance();
        }
    }

    fn number(&mut self) -> Token {
        let mut lexeme = String::new();
        let mut token_type = TokenType::INTEGER;

        while self.peek().is_digit(10) {
            lexeme.push(self.peek());
            self.advance();
        }

        if self.peek() == '.' {
            token_type = TokenType::FLOAT;
            lexeme.push(self.peek());
            self.advance();

            while self.peek().is_digit(10) {
                lexeme.push(self.peek());
                self.advance();
            }
        }

        Token::new(token_type, lexeme)
    }

    pub fn tokenize(&mut self) -> Vec<Token> {
        let mut tokens: Vec<Token> = Vec::new();
        while self.peek() != '\0' {
            match self.peek() {
                '0'..='9' => {
                    let token: Token = self.number();
                    tokens.push(token)
                },
                ';' => tokens.push(Token::new(TokenType::SEMICOLON, ";".to_string())),
                '+' => tokens.push(Token::new(TokenType::PLUS, "+".to_string())),
                '-' => tokens.push(Token::new(TokenType::MINUS, "-".to_string())),
                _ => self.skip_whitespace()
            }

            self.advance();
        }

        tokens.push(Token::new(TokenType::EOF, String::from("EOF")));

        tokens
    }
}

// impl Debug for Lexer {
//     fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
//         f.debug_struct("Lexer")
//             .field("input", &self.input)
//             .field("position", &self.position)
//             .field("line", &self.line)
//             .field("tokens", &self.tokens)
//             .finish()
//     }
// }