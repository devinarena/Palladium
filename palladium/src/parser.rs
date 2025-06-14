use std::{time::{Duration, Instant}};

use crate::{syntax_tree::{ExpressionNode, MainNode, StatementNode}, token::Token};

macro_rules! parse_error {
    ($($arg:tt)*) => {
        panic!("[Yttrium] parse error: {}", $($arg)*);
    };
}

pub struct Parser<'a> {
    tokens: &'a Vec<Token>,
    index: usize,
    pub parse_time: Duration,
}

impl Parser<'_> {
    pub fn new(tokens: &Vec<Token>) -> Parser {
        Parser { 
            tokens, 
            index: 0 ,
            parse_time: Duration::new(0, 0)
        }
    }

    pub fn parse(&mut self) -> MainNode {
        let start_time = Instant::now();
        let mut program = MainNode::new();
        while !matches!(self.peek(), Token::EndOfFile) {
            self.statement(&mut program);
        }
        self.parse_time = start_time.elapsed();
        program
    }

    pub fn consume(&mut self) -> &Token {
        let token = self.tokens.get(self.index);
        if token.is_none() {
            return &Token::EndOfFile;
        }
        self.index += 1;
        return token.unwrap();
    }

    pub fn peek(&self) -> &Token {
        let token: Option<&Token> = self.tokens.get(self.index);
        if token.is_none() {
            return &Token::EndOfFile;
        }
        return token.unwrap();
    }

    fn expression(&mut self, min_bp: u8) -> ExpressionNode {
        let mut lhs = match self.peek() {
            Token::StringLiteral(_) | Token::Decimal(_) => ExpressionNode::Literal { value_token:  Box::new(self.consume().clone()) },
            Token::Identifier(_) => ExpressionNode::Variable { identifier: self.consume().get_value() },
            _ => {
                parse_error!("Expected expression");
            }
        };
        loop {
            let op = self.peek().clone();
            if !matches!(op, Token::Plus | Token::Minus | Token::Star | Token::Slash) {
                break;
            }
            self.consume();

            let (lbp, rbp) = self.infix_binding_power(&op);
            if lbp < min_bp {
                break;
            }

            let rhs = self.expression(rbp);

            match op {
                Token::Plus | Token::Minus | Token::Star | Token::Slash => {
                    lhs = ExpressionNode::Binary { left: Box::new(lhs), operator: Box::new(op), right: Box::new(rhs) };
                }
                _ => parse_error!(format!("Unexpected operator: {:?}", op)),
            }
        }
        lhs
    }

    fn infix_binding_power(&self, op: &Token) -> (u8, u8) {
        match op {
            Token::Plus | Token::Minus => (1, 2),
            Token::Star | Token::Slash => (3, 4),
            _ => panic!("bad op: {:?}", op),
        }
    }

    fn statement(&mut self, program: &mut MainNode) {
        match self.peek() {
            Token::Output => {
                self.consume();
                self.output_statement(program);
            }
            Token::Let => {
                self.consume();
                self.let_statement(program);
            }
            _ => {
                parse_error!("Expected statement");
            }
        }
    }

    fn let_statement(&mut self, program: &mut MainNode) {
        if !matches!(self.peek(), Token::Identifier(_)) {
            parse_error!("Expected identifier after 'let'");
        }
        let identifier = self.consume().get_value();
        for _ in 0..2 {
            if !matches!(self.consume(), Token::Colon) {
                parse_error!("Expected '::' followed by type declaration");
            }
        }
        let type_token = self.consume().clone();
        if !matches!(type_token, Token::F32) {
            parse_error!("Expected type declaration (currently supported: f32)");
        }
        if !matches!(self.peek(), Token::Equals) {
            parse_error!("Expected '=' after identifier");
        }
        self.consume(); // consume the equals sign
        let expression_node = self.expression(0);
        let let_node = StatementNode::Let {
            identifier,
            type_token: type_token,
            expression: expression_node,
        };
        program.add_child(let_node);
    }

    fn output_statement(&mut self, program: &mut MainNode) {
        if !matches!(self.peek(), Token::LeftParen) {
            parse_error!(format!("Expected left parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the left parenthesis
        let expression_node = self.expression(0);
        let output_node = StatementNode::Output { expression: expression_node };
        program.add_child(output_node);
        if !matches!(self.peek(), Token::RightParen) {
            parse_error!(format!("Expected right parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the right parenthesis
    }
}