use std::{collections::HashMap, time::{Duration, Instant}};

use crate::{syntax_tree::{ExpressionNode, ExpressionNodeType, MainNode, StatementNode, ValueType}, token::{Token, TokenType}};

macro_rules! parse_error {
    ($line:expr, $($arg:tt)*) => {
        panic!("[Palladium] parse error (line {}): {}", $line, $($arg)*)
    };
}

pub struct Scope {
    identifiers: HashMap<String, ValueType>,
    parent: Option<Box<Scope>>
}

pub struct Parser<'a> {
    tokens: &'a Vec<Token>,
    index: usize,
    pub file_name: String,
    pub parse_time: Duration,
    scope: Scope
}

impl Parser<'_> {
    pub fn new(file_name: String, tokens: &Vec<Token>) -> Parser {
        Parser { 
            tokens, 
            index: 0 ,
            file_name,
            scope: Scope { parent: None, identifiers: HashMap::new() },
            parse_time: Duration::new(0, 0)
        }
    }

    pub fn parse(&mut self) -> StatementNode {
        let start_time = Instant::now();
        let mut program = StatementNode::Main { body: Box::new(StatementNode::Block { children: Vec::new() }) };
        while !matches!(self.peek().token_type, TokenType::EndOfFile) {
            self.statement(&mut program);
        }
        self.parse_time = start_time.elapsed();
        program
    }

    pub fn consume(&mut self) -> &Token {
        let token = self.tokens.get(self.index);
        if token.is_none() {
            return self.tokens.last().unwrap(); // should be end of file
        }
        self.index += 1;
        return token.unwrap();
    }

    pub fn peek(&self) -> &Token {
        let token: Option<&Token> = self.tokens.get(self.index);
        if token.is_none() {
            return self.tokens.last().unwrap();
        }
        return token.unwrap();
    }

    fn lookup_value(&self, name: &String, scope: &Scope) -> ValueType {
        if let Some(value) = scope.identifiers.get(name) {
            return value.clone();
        }
        if let Some(parent) = &scope.parent {
            return self.lookup_value(name, parent);
        }
        panic!("Could not find variable {}", name);
    }


    fn block(&mut self) -> StatementNode {
        let mut block = StatementNode::Block { children: Vec::new() };
        self.consume(); // {
        while !matches!(self.peek().token_type, TokenType::RightBrace) {
            self.statement(&mut block);
        }
        self.consume(); // }
        block
    }


    fn expression(&mut self, min_bp: u8) -> ExpressionNode {
        let mut lhs = match self.peek().token_type {
            TokenType::StringLiteral(_)=> ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::String ),
            TokenType::Decimal(_) => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Float ),
            TokenType::True | TokenType::False => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Boolean ),
            TokenType::Identifier(ref name) => ExpressionNode::new(ExpressionNodeType::Variable { 
                identifier: self.peek().get_value() }, 
                self.lookup_value(name, &self.scope)
            ),
            TokenType::LeftParen => {
                self.consume();
                let expr = self.expression(0);
                if !matches!(self.peek().token_type, TokenType::RightParen) {
                    parse_error!(self.peek().line_number, format!("Expected right parenthesis but got {:?}", self.peek()).as_str());
                }
                // right parenthesis is consumed in self.expression
                expr
            }
            _ => {
                parse_error!(self.peek().line_number, "Expected expression");
            }
        };
        self.consume();
        loop {
            let op = self.peek().clone();
            if !matches!(op.token_type, TokenType::Plus | TokenType::Minus | TokenType::Star | TokenType::Slash | TokenType::And
                | TokenType::Or | TokenType::GreaterThan | TokenType::LessThan | TokenType::GreaterEqualTo | TokenType::LessEqualTo | TokenType::DoubleEquals) {
                break;
            }

            let (lbp, rbp) = self.infix_binding_power(&op);
            if lbp < min_bp {
                break;
            }
            
            self.consume();

            let rhs = self.expression(rbp);

            match op.token_type {
                TokenType::Plus | TokenType::Minus | TokenType::Star | TokenType::Slash => {
                    let mut value_type: ValueType = ValueType::Float;
                    if lhs.value_type == ValueType::String {
                        value_type = ValueType::String;
                    } else if rhs.value_type == ValueType::String {
                        value_type = ValueType::String;
                    }
                    lhs = ExpressionNode::new(ExpressionNodeType::Binary { left: Box::new(lhs), operator: Box::new(op), right: Box::new(rhs), }, value_type );
                }
                TokenType::GreaterThan | TokenType::LessThan | TokenType::GreaterEqualTo | TokenType::LessEqualTo => {
                    if lhs.value_type != ValueType::Float {
                        parse_error!(self.peek().line_number, "Expected float expression on left side of comparison");
                    } else if rhs.value_type != ValueType::Float {
                        parse_error!(self.peek().line_number, "Expected float expression on right side of comparison");
                    }
                    lhs = ExpressionNode::new(ExpressionNodeType::Binary { left: Box::new(lhs), operator: Box::new(op), right: Box::new(rhs), }, ValueType::Boolean );
                }
                TokenType::DoubleEquals => {
                    // gonna have to figure out how to not break this in Java
                    lhs = ExpressionNode::new(ExpressionNodeType::Binary { left: Box::new(lhs), operator: Box::new(op), right: Box::new(rhs), }, ValueType::Boolean );
                }
                TokenType::And | TokenType::Or => {
                    if lhs.value_type != ValueType::Boolean {
                        parse_error!(self.peek().line_number, "Expected boolean expression on left side of '&&' or '||'");
                    } else if rhs.value_type != ValueType::Boolean {
                        parse_error!(self.peek().line_number, "Expected boolean expression on right side of '&&' or '||'");
                    }
                    lhs = ExpressionNode::new(ExpressionNodeType::Binary { left: Box::new(lhs), operator: Box::new(op), right: Box::new(rhs), }, ValueType::Boolean );
                }
                _ => parse_error!(self.peek().line_number, format!("Unexpected operator: {:?}", op)),
            }
        }
        lhs
    }

    fn infix_binding_power(&self, op: &Token) -> (u8, u8) {
        match op.token_type {
            TokenType::Or => (0, 1),
            TokenType::And => (2, 3),
            TokenType::GreaterThan | TokenType::LessThan | TokenType::GreaterEqualTo | TokenType::LessEqualTo | TokenType::DoubleEquals => (4, 5),
            TokenType::Plus | TokenType::Minus => (6, 7),
            TokenType::Star | TokenType::Slash => (8, 9),
            _ => panic!("bad op: {:?}", op),
        }
    }

    fn statement(&mut self, program: &mut StatementNode) {
        match self.peek().token_type {
            TokenType::Output => {
                self.consume();
                self.output_statement(program);
            }
            TokenType::Let => {
                self.consume();
                self.let_statement(program);
            }
            TokenType::Loop => {
                self.consume();
                self.loop_statement(program);
            }
            _ => {
                parse_error!(self.peek().line_number, "Expected statement");
            }
        }
    }


    fn loop_statement(&mut self, program: &mut StatementNode) {
        let body = self.block();
        let loop_node = StatementNode::Loop { body: Box::new(body) };
        program.add_child(loop_node);
    }


    fn let_statement(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
            parse_error!(self.peek().line_number, "Expected identifier after 'let'");
        }
        let identifier = self.consume().get_value();
        for _ in 0..2 {
            if matches!(self.peek().token_type, TokenType::Colon) {
                self.consume();
            } else {
                parse_error!(self.peek().line_number, "Expected '::' followed by type declaration");
            }
        }
        let type_token = self.peek().clone();
        if matches!(type_token.token_type, TokenType::F32 | TokenType::Str | TokenType::Bool) {
            self.consume();
        } else {
            parse_error!(self.peek().line_number, "Expected type declaration (currently supported: f32, str, bool)");
        }
        if matches!(self.peek().token_type, TokenType::Equals) {
            self.consume(); // consume the equals sign
        } else{ 
            parse_error!(self.peek().line_number, "Expected '=' after identifier");
        }
        let expression_node = self.expression(0);
        if expression_node.value_type != type_token.get_value_type_declaration() {
            parse_error!(self.peek().line_number, format!("Expected expression of type {:?} but got expression of type {:?}", type_token.get_value_type_declaration(), expression_node.value_type).as_str());
        }
        let declared_type = type_token.get_value_type_declaration();
        let let_node = StatementNode::Let {
            identifier: identifier.clone(),
            type_token: type_token,
            expression: expression_node,
        };
        program.add_child(let_node);
        self.scope.identifiers.insert(identifier, declared_type);
    }


    fn output_statement(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::LeftParen) {
            parse_error!(self.peek().line_number, format!("Expected left parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the left parenthesis
        let expression_node = self.expression(0);
        let output_node = StatementNode::Output { expression: expression_node };
        program.add_child(output_node);
        if !matches!(self.peek().token_type, TokenType::RightParen) {
            parse_error!(self.peek().line_number, format!("Expected right parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the right parenthesis
    }
}