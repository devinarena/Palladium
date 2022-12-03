use std::{fmt::Debug, fs};

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

    fn match_char(&mut self, expected: char) -> bool {
        if self.peek() == expected {
            self.advance();
            return true;
        }

        false
    }

    fn consume(&mut self, expected: char, message: &str) -> bool {
        if self.peek() == expected {
            self.advance();
            return true;
        }

        panic!("[line {}] Error: {} at position {}", self.line, message, self.position);
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

        Token::new(token_type, lexeme, self.line)
    }

    fn string(&mut self) -> Token {
        let mut lexeme = String::new();
        let line = self.line;
        self.advance();

        while self.peek() != '"' {

            if self.peek() == '\0' {
                panic!("Unterminated string");
            }

            if self.peek() == '\n' {
                self.line += 1;
            }

            lexeme.push(self.peek());

            self.advance();
        }

        self.consume('"', "Unterminated string");

        Token::new(TokenType::STRING, lexeme, line)
    }

    pub fn tokenize(&mut self) -> Vec<Token> {
        let mut tokens: Vec<Token> = Vec::new();
        while self.peek() != '\0' {
            let mut advance: bool = true;
            match self.peek() {
                '0'..='9' => {
                    let token: Token = self.number();
                    advance = false;
                    tokens.push(token)
                }
                ';' => tokens.push(Token::new(TokenType::SEMICOLON, ";".to_string(), self.line)),
                '+' => tokens.push(Token::new(TokenType::PLUS, "+".to_string(), self.line)),
                '-' => tokens.push(Token::new(TokenType::MINUS, "-".to_string(), self.line)),
                '*' => tokens.push(Token::new(TokenType::STAR, "*".to_string(), self.line)),
                '/' => tokens.push(Token::new(TokenType::SLASH, "/".to_string(), self.line)),
                '(' => tokens.push(Token::new(
                    TokenType::LEFT_PAREN,
                    "(".to_string(),
                    self.line,
                )),
                ')' => tokens.push(Token::new(
                    TokenType::RIGHT_PAREN,
                    ")".to_string(),
                    self.line,
                )),
                '"' => {
                    let string: Token = self.string();
                    advance = false;
                    tokens.push(string)
                }
                _ => self.skip_whitespace(),
            }
            if advance {
                self.advance();
            }
        }

        tokens.push(Token::new(TokenType::EOF, String::from("EOF"), self.line));

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
