use crate::expression::expression::Expression;
use crate::expression::statement::Statement;
use crate::token::{Token, TokenType};
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

    fn match_token(&mut self, token_type: TokenType) -> bool {
        if self.is_at_end() {
            return false;
        }

        if self.peek().token_type != token_type {
            return false;
        }

        self.advance();
        true
    }

    fn consume(&mut self, token_type: TokenType, message: &str) {
        if self.match_token(token_type) {
            return;
        }

        panic!("{}", message);
    }

    fn literal(&mut self) -> Expression {
        self.advance();

        match self.previous().token_type {
            TokenType::INTEGER_LITERAL => Expression::Literal(Value::Integer(
                self.previous().lexeme.parse::<i64>().unwrap(),
            )),
            TokenType::FLOAT_LITERAL => Expression::Literal(Value::Double(
                self.previous().lexeme.parse::<f64>().unwrap(),
            )),
            TokenType::STRING_LITERAL => Expression::Literal(Value::String(self.previous().lexeme)),
            TokenType::IDENTIFIER => Expression::Variable(self.previous().lexeme),
            TokenType::LEFT_PAREN => {
                let expr = self.expression();
                self.consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
                expr
            }
            _ => panic!("Unexpected token type"),
        }
    }

    fn unary(&mut self) -> Expression {
        if self.match_token(TokenType::MINUS) {
            return Expression::Unary(TokenType::MINUS, Box::new(self.literal()));
        }
        self.literal()
    }

    fn term(&mut self) -> Expression {
        let mut expr = self.unary();

        while self.match_token(TokenType::STAR) || self.match_token(TokenType::SLASH) {
            let operator = self.previous();
            let right = self.unary();
            expr = Expression::Binary(operator.token_type, Box::new(expr), Box::new(right));
        }

        expr
    }

    fn primary(&mut self) -> Expression {
        let mut expr = self.term();

        while self.match_token(TokenType::PLUS) || self.match_token(TokenType::MINUS) {
            let operator = self.previous();
            let right = self.term();
            expr = Expression::Binary(operator.token_type, Box::new(expr), Box::new(right));
        }

        expr
    }

    pub fn expression(&mut self) -> Expression {
        self.primary()
    }

    pub fn print_statement(&mut self) -> Statement {
        let value = self.expression();
        self.consume(TokenType::SEMICOLON, "Expected ';' after value");
        Statement::PrintStatement(Box::new(value))
    }

    pub fn expression_statement(&mut self) -> Statement {
        let expr = self.expression();
        self.consume(TokenType::SEMICOLON, "Expected ';' after expression");
        Statement::ExpressionStatement(Box::new(expr))
    }

    pub fn statement(&mut self) -> Statement {
        if self.match_token(TokenType::PRINT) {
            return self.print_statement();
        }

        self.expression_statement()
    }

    pub fn parse(&mut self) -> Vec<Statement> {
        let mut statements = Vec::new();

        while !self.is_at_end() {
            statements.push(self.statement());
        }

        statements
    }
}
