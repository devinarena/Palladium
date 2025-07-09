use std::{collections::HashMap, time::{Duration, Instant}};

use crate::{syntax_tree::{ExpressionNode, ExpressionNodeType, StatementNode, ValueType}, token::{Token, TokenType}};

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
    scope: Scope,
    loop_counter: usize,
}

impl Parser<'_> {
    pub fn new(file_name: String, tokens: &Vec<Token>) -> Parser {
        Parser { 
            tokens, 
            index: 0 ,
            file_name,
            scope: Scope { parent: None, identifiers: HashMap::new() },
            parse_time: Duration::new(0, 0),
            loop_counter: 0
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


    pub fn look_ahead(&self, offset: usize) -> &Token {
        let index = self.index + offset;
        if index >= self.tokens.len() {
            return self.tokens.last().unwrap();
        }
        return &self.tokens[index];
    }


    fn lookup_value(&self, name: &String, scope: &Scope) -> Option<ValueType> {
        if let Some(value) = scope.identifiers.get(name) {
            return Some(value.clone());
        }
        if let Some(parent) = &scope.parent {
            return self.lookup_value(name, parent);
        }
        None
    }


    fn new_scope(&mut self) {
        let new_scope = Scope {
            identifiers: HashMap::new(),
            parent: Some(Box::new(std::mem::replace(&mut self.scope, Scope { identifiers: HashMap::new(), parent: None})))
        };
        self.scope = new_scope;
    }


    fn pop_scope(&mut self) {
        if let Some(parent) = self.scope.parent.take() {
            self.scope = *parent;
        } else {
            parse_error!(self.peek().line_number, "No parent scope to pop");
        }   
    }


    fn block(&mut self) -> StatementNode {
        let mut block = StatementNode::Block { children: Vec::new() };
        self.new_scope();
        self.consume(); // {
        while !matches!(self.peek().token_type, TokenType::RightBrace) {
            self.statement(&mut block);
        }
        self.consume(); // }
        self.pop_scope();
        block
    }


    fn unscoped_block(&mut self) -> StatementNode {
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
            TokenType::Integer(_) => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Integer ),
            TokenType::True | TokenType::False => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Boolean ),
            TokenType::Identifier(ref name) => ExpressionNode::new(ExpressionNodeType::Variable { 
                identifier: self.peek().get_value() }, 
                self.lookup_value(name, &self.scope).unwrap_or_else(|| {
                    parse_error!(self.peek().line_number, format!("Unknown identifier: {}", name).as_str());
                })
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
                    if lhs.value_type == ValueType::Integer && rhs.value_type == ValueType::Integer {
                        value_type = ValueType::Integer;
                    } else if lhs.value_type == ValueType::String || rhs.value_type == ValueType::String {
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
            TokenType::If => {
                self.consume();
                self.if_statement(program);
            }
            TokenType::Identifier(_) => {
                self.assignment_statement(program);
            }
            TokenType::Break => {
                self.consume();
                program.add_child(StatementNode::Break);
            }
            _ => {
                parse_error!(self.peek().line_number, "Expected statement");
            }
        }
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
        if matches!(type_token.token_type, TokenType::F32 | TokenType::I32 | TokenType::Str | TokenType::Bool) {
            self.consume();
        } else {
            parse_error!(self.peek().line_number, "Expected type declaration (currently supported: f32, i32, str, bool)");
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

    fn loop_statement(&mut self, program: &mut StatementNode) {
        let mut range: Option<ExpressionNode> = None;
        let mut body = None;
        if !matches!(self.peek().token_type, TokenType::LeftBrace) {
            let start = self.expression(0);
            let expected_type =start.value_type.clone();
            if expected_type != ValueType::Float && expected_type != ValueType::Integer {
                parse_error!(self.peek().line_number, "Expected float or int expression in loop condition");
            }
            if !matches!(self.peek().token_type, TokenType::DoubleDot) {
                parse_error!(self.peek().line_number, format!("Expected '..' after loop condition but got {:?}", self.peek()).as_str());
            }
            self.consume(); // consume the double dot
            let end = self.expression(0);
            if end.value_type != expected_type {
                parse_error!(self.peek().line_number, "Expected same type expression in loop condition");
            }
            let identifier = if matches!(self.peek().token_type, TokenType::As) {
                self.consume(); // consume the 'as' keyword
                if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
                    parse_error!(self.peek().line_number, "Expected identifier after 'as'");
                }
                self.consume().get_value()
            } else {
                format!("__palladium__loop__{}", self.loop_counter)
            };
            range = Some(ExpressionNode::new(ExpressionNodeType::Range { 
                identifier: identifier.clone(),
                range_type: expected_type.clone(), 
                start: Box::new(start), 
                end: Box::new(end) 
            }, expected_type.clone()));
            self.loop_counter += 1;
            self.new_scope();
            self.scope.identifiers.insert(identifier, expected_type);
            body = Some(self.unscoped_block());
            self.pop_scope();
        } else {
            body = Some(self.block());
        }
        let loop_node = StatementNode::Loop { range, body: Box::new(body.expect("Expected loop body")) };
        program.add_child(loop_node);
    }
    

    fn if_statement(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::LeftParen) {
            parse_error!(self.peek().line_number, format!("Expected left parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the left parenthesis
        let condition = self.expression(0);
        if condition.value_type != ValueType::Boolean {
            parse_error!(self.peek().line_number, "Expected boolean expression in if condition");
        }
        if !matches!(self.peek().token_type, TokenType::RightParen) {
            parse_error!(self.peek().line_number, format!("Expected right parenthesis but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the right parenthesis
        let if_body = self.block();
        let else_body = if matches!(self.peek().token_type, TokenType::Else) {
            self.consume();
            Some(Box::new(self.block()))
        } else {
            None
        };
        let if_node = StatementNode::If { condition, body: Box::new(if_body), else_body: else_body };
        program.add_child(if_node);
    }

    fn assignment_statement(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
            parse_error!(self.peek().line_number, "Expected identifier for assignment");
        }
        let identifier = self.consume().get_value();
        if !matches!(self.peek().token_type, TokenType::Equals) {
            parse_error!(self.peek().line_number, format!("Expected '=' after identifier but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the equals sign
        let expression_node = self.expression(0);
        let value_type = self.lookup_value(&identifier, &self.scope);
        if value_type.is_none() {
            // todo: write lookup function
            parse_error!(self.peek().line_number, format!("Unknown identifier: {}", identifier).as_str());
        } else if Some(expression_node.value_type.clone()) != value_type {
            parse_error!(self.peek().line_number, format!("Expected expression of type {:?} but got expression of type {:?}", value_type, expression_node.value_type).as_str());
        }
        let assignment_node = StatementNode::Assignment { identifier: identifier.clone(), expression: expression_node };
        program.add_child(assignment_node);
    }
}