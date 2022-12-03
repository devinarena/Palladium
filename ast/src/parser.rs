use crate::token::{Token, TokenType};
use crate::expression::expression::Expression;
use crate::value::Value;


#[derive(Debug)]
pub struct Parser {
    tokens: Vec<Token>,
    position: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Parser {
        Parser {
            tokens,
            position: 0,
        }
    }

    fn parse_error(&self, message: &str) {
        panic!("[line {}] Parse error: {} at instruction {}", self.peek().line, message, self.position);
    }

    fn previous(&self) -> Token {
        if self.position == 0 {
            self.tokens[self.position].clone()
        } else {
            self.tokens[self.position - 1].clone()
        }
    }

    fn peek(&self) -> &Token {
        if self.position >= self.tokens.len() {
            &self.tokens[self.tokens.len() - 1]
        } else {
            &self.tokens[self.position]
        }
    }
    
    fn peek_next(&self) -> &Token {
        if self.position + 1 >= self.tokens.len() {
            &self.tokens[self.tokens.len() - 1]
        } else {
            &self.tokens[self.position + 1]
        }
    }

    fn advance(&mut self) {
        self.position += 1;
    }

    fn check(&mut self, token_type: TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }

        self.peek().token_type == token_type
    }

    fn match_token(&mut self, expected: TokenType) -> bool {
        if self.check(expected) {
            self.advance();
            true
        } else {
            false
        }
    }

    fn consume(&mut self, expected: TokenType, message: &str) -> Token {
        if !self.match_token(expected) {
            self.parse_error(message);
            return Token::new(TokenType::ERROR, String::new(), 0);
        }

        self.advance();
        self.previous()
    }

    fn is_at_end(&self) -> bool {
        self.peek().token_type == TokenType::EOF
    }

    fn literal(&mut self) -> Expression {
        self.advance();

        match self.previous().token_type {
            TokenType::INTEGER => {
                Expression::Literal(Value::Integer(self.previous().lexeme.parse::<i32>().unwrap()))
            },
            TokenType::FLOAT => {
                Expression::Literal(Value::Float(self.previous().lexeme.parse::<f32>().unwrap()))
            },
            _ => panic!("Unexpected token type")
        }
    }

    fn unary(&mut self) -> Expression {
        while self.match_token(TokenType::MINUS) {
            let operator = self.previous().token_type;
            let right = self.unary();

            return Expression::Unary(operator, Box::new(right));
        }

        self.literal()
    }

    fn binary(&mut self) -> Expression {
        let mut expression = self.unary();

        println!("{}, {}", self.peek().token_type, self.peek_next().token_type);

        while self.match_token(TokenType::PLUS) || self.match_token(TokenType::MINUS) {
            let operator = self.previous().token_type;
            let right = self.unary();

            expression = Expression::Binary(Box::new(expression), operator, Box::new(right));
        }

        expression
    }

    pub fn expression(&mut self) -> Expression {
        return self.binary();
    }
}