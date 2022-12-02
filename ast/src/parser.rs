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

    fn previous(&self) -> Token {
        if self.position == 0 {
            self.tokens[self.position].clone()
        } else {
            self.tokens[self.position - 1].clone()
        }
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.position]
    }
    
    fn peek_next(&self) -> &Token {
        &self.tokens[self.position + 1]
    }

    fn advance(&mut self) {
        self.position += 1;
    }

    fn is_at_end(&self) -> bool {
        self.peek().token_type == TokenType::EOF
    }

    fn literal(&mut self) -> Expression {
        match self.previous().token_type {
            TokenType::INTEGER => {
                Expression::Literal(Value::Integer(self.previous().lexeme.parse::<i32>().unwrap()))
            },
            _ => panic!("Unexpected token type")
        }
    }

    pub fn expression(&mut self) -> Expression {
        self.advance();

        self.literal()
    }
}