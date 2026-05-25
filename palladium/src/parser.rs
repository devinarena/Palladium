use std::{collections::HashMap, sync::Arc, time::{Duration, Instant}};

use crate::{builtins::Builtin, syntax_tree::{ExpressionNode, ExpressionNodeType, StatementNode, ValueType}, token::{Token, TokenType}};

macro_rules! parse_error {
    ($line:expr, $($arg:tt)*) => {
        panic!("[Palladium] parse error (line {}): {}", $line, $($arg)*)
    };
}



pub struct Scope {
    functions: Option<HashMap<String, ValueType>>,
    identifiers: HashMap<String, ValueType>,
    parent: Option<Box<Scope>>
}

pub struct Parser<'a> {
    tokens: &'a Vec<Token>,
    index: usize,
    pub file_context: FileContext,
    builtins: Arc<HashMap<String, Builtin>>,
    pub parse_time: Duration,
    scope: Scope,
    loop_counter: usize,
}

pub struct FileContext {
    pub imports: Vec<String>,
    pub main_class: Option<StatementNode>,
    pub body: Option<StatementNode>,
    pub file_name: String,
}

impl Parser<'_> {
    pub fn new(file_name: String, tokens: &Vec<Token>, builtins: Arc<HashMap<String, Builtin>>) -> Parser {
        let mut identifiers = HashMap::new();
        for (name, builtin) in builtins.iter() {
            identifiers.insert(name.clone(), builtin.return_type.clone());
        }
        Parser { 
            tokens, 
            index: 0 ,
            file_context: FileContext {
                imports: Vec::new(),
                main_class: None,
                body: None,
                file_name: file_name
            },
            builtins,
            scope: Scope { 
                parent: None,
                functions: Some(HashMap::new()),
                identifiers
            },
            parse_time: Duration::new(0, 0),
            loop_counter: 0
        }
    }

    pub fn parse(&mut self) {
        let start_time = Instant::now();
        let mut program = StatementNode::Block { children: Vec::new() };
        while !matches!(self.peek().token_type, TokenType::EndOfFile) {
            self.statement(&mut program);
        }
        if let StatementNode::Block { ref children } = program {
            let main = children.iter().find(|node| matches!(node, StatementNode::Main { .. }));
            let class = children.iter().find(|node| matches!(node, StatementNode::Class { .. }));
            // if theres no class declaration, we need to create one to compile to Java
            if class.is_none() {
                // if theres no main node but there is statement nodes, we need to create a main node and move all the statements into it
                if main.is_none() {

                } else {
                    program = StatementNode::Block { children: vec![main.unwrap().clone()] };
                }
            }
        }

        self.parse_time = start_time.elapsed();
        self.file_context.body = Some(program);
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


    fn lookup_function(&self, name: &String, scope: &Scope) -> Option<ValueType> {
        if let Some(functions) = &scope.functions {
            if let Some(value) = functions.get(name) {
                return Some(value.clone());
            }
        }
        if let Some(parent) = &scope.parent {
            return self.lookup_function(name, parent);
        }
        if self.builtins.contains_key(name) {
            return Some(self.builtins.get(name).unwrap().return_type.clone());
        }
        return None;
    }


    fn new_scope(&mut self) {
        let new_scope = Scope {
            identifiers: HashMap::new(),
            functions: None,
            parent: Some(Box::new(std::mem::replace(&mut self.scope, Scope { identifiers: HashMap::new(), functions: None, parent: None})))
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

    fn add_import(&mut self, import: String) {
        if !self.file_context.imports.contains(&import) {
            self.file_context.imports.push(import);
        }
    }

    fn parse_call(&mut self, callee: ExpressionNode) -> ExpressionNode {
        // consume '('
        self.consume();

        let mut args = Vec::new();

        if !matches!(self.peek().token_type, TokenType::RightParen) {
            loop {
                args.push(self.expression(0));

                if matches!(self.peek().token_type, TokenType::Comma) {
                    self.consume();
                } else {
                    break;
                }
            }
        }

        if !matches!(self.peek().token_type, TokenType::RightParen) {
            parse_error!(self.peek().line_number, "Expected ')'");
        }

        self.consume(); // consume ')'

        if let ExpressionNodeType::Variable { ref identifier } = callee.node_type {
            if self.builtins.contains_key(identifier) {
                if identifier == "input" {
                    self.add_import("java.util.Scanner".to_string());
                }
                let builtin = self.builtins.get(identifier).unwrap();
                if args.len() != builtin.args.len() {
                    parse_error!(self.peek().line_number, format!("Expected {} arguments but got {}", builtin.args.len(), args.len()).as_str());
                }
                return ExpressionNode::new(
                    ExpressionNodeType::FunctionCall {
                        callee: Box::new(callee),
                        arguments: args,
                        return_type: builtin.return_type.clone()
                    },
                    ValueType::Function // or function return type
                );
            } else {
                let value_type = self.lookup_function(identifier, &self.scope);
                if value_type.is_none() {
                    parse_error!(self.peek().line_number, format!("Unknown function: {}", identifier).as_str());
                }
                return ExpressionNode::new(
                    ExpressionNodeType::FunctionCall {
                        callee: Box::new(callee),
                        arguments: args,
                        return_type: ValueType::Function // we don't actually know the return type here, but we'll figure it out in the compiler
                    },
                    ValueType::Function
                );
            }
        }

        parse_error!(self.peek().line_number, "Can only call functions");
    }

    fn expression(&mut self, min_bp: u8) -> ExpressionNode {
        let mut lhs = match self.peek().token_type {
            TokenType::StringLiteral(_)=> ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::String ),
            TokenType::Decimal(_) => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Float ),
            TokenType::Integer(_) => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Integer ),
            TokenType::True | TokenType::False => ExpressionNode::new(ExpressionNodeType::Literal { value_token: Box::new(self.peek().clone()) }, ValueType::Boolean ),
            TokenType::Identifier(ref name) => ExpressionNode::new(ExpressionNodeType::Variable { identifier: self.peek().get_value() }, 
                self.lookup_value(name, &self.scope).unwrap_or_else(|| {
                    self.lookup_function(name, &self.scope).unwrap_or_else(|| {
                        parse_error!(self.peek().line_number, format!("Unknown identifier: {}", name).as_str());
                    })
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
            match self.peek().token_type {
                TokenType::LeftParen => {
                    lhs = self.parse_call(lhs);
                    continue;
                }
                TokenType::LeftBrace => {
                    // Object instantiation
                    if let ExpressionNodeType::Variable { ref identifier } = lhs.node_type {
                        let class_name = identifier.clone();
                        self.consume(); // consume {
                        let mut fields = Vec::new();
                        
                        if !matches!(self.peek().token_type, TokenType::RightBrace) {
                            loop {
                                if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
                                    parse_error!(self.peek().line_number, "Expected field name");
                                }
                                let field_name = self.consume().get_value();
                                if !matches!(self.peek().token_type, TokenType::Colon) {
                                    parse_error!(self.peek().line_number, "Expected ':' after field name");
                                }
                                self.consume(); // consume :
                                let field_value = self.expression(0);
                                fields.push((field_name, field_value));
                                
                                if matches!(self.peek().token_type, TokenType::Comma) {
                                    self.consume();
                                } else {
                                    break;
                                }
                            }
                        }
                        
                        if !matches!(self.peek().token_type, TokenType::RightBrace) {
                            parse_error!(self.peek().line_number, "Expected '}'");
                        }
                        self.consume(); // consume }
                        
                        // For now, use a placeholder value type - should be the class type
                        lhs = ExpressionNode::new(
                            ExpressionNodeType::ObjectLiteral { class_name, fields },
                            ValueType::Function // placeholder, will be improved later
                        );
                    } else {
                        parse_error!(self.peek().line_number, "Cannot instantiate non-class");
                    }
                    continue;
                }
                TokenType::Dot => {
                    // Member access
                    self.consume(); // consume .
                    if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
                        parse_error!(self.peek().line_number, "Expected identifier after '.'");
                    }
                    let member = self.consume().get_value();
                    
                    // For now, use placeholder value type
                    lhs = ExpressionNode::new(
                        ExpressionNodeType::MemberAccess {
                            object: Box::new(lhs),
                            member
                        },
                        ValueType::Function // placeholder
                    );
                    continue;
                }
                _ => {}
            }
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
                    if self.check_comparison_types(&lhs.value_type, &rhs.value_type) == false {
                        parse_error!(self.peek().line_number, "Mismatched types in comparison expression");
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

    fn check_comparison_types(&self, left: &ValueType, right: &ValueType) -> bool {
        match left {
            ValueType::Float => matches!(right, ValueType::Float),
            ValueType::Integer => matches!(right, ValueType::Integer),
            ValueType::String => matches!(right, ValueType::String),
            ValueType::Boolean => matches!(right, ValueType::Boolean),
            _ => false
        }
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
                if matches!(self.look_ahead(1).token_type, TokenType::LeftParen) {
                    self.call_statement(program);
                } else {
                    self.assignment_statement(program);
                }
            }
            TokenType::Break => {
                self.consume();
                program.add_child(StatementNode::Break);
            }
            TokenType::Return => {
                self.consume();
                let expression = self.expression(0);
                program.add_child(StatementNode::Return { expression });
            },
            TokenType::Class => {
                self.consume();
                self.class_declaration(program);
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
        if matches!(type_token.token_type, TokenType::Fn) {
            self.function_declaration(program, &identifier);
        } else {
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
            if expression_node.value_type == ValueType::Function {
                if let ExpressionNodeType::FunctionCall { ref return_type, .. } = expression_node.node_type {
                    if return_type != &type_token.get_value_type_declaration() {
                        parse_error!(self.peek().line_number, format!("Expected function to return type {:?} but got function that returns type {:?}", type_token.get_value_type_declaration(), return_type).as_str());
                    }
                } else {
                    parse_error!(self.peek().line_number, "Expected function call expression");
                }
            } else if expression_node.value_type != type_token.get_value_type_declaration() {
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
    }

    fn function_declaration(&mut self, program: &mut StatementNode, identifier: &String) {
        if !matches!(program, StatementNode::Class { .. }) && self.scope.parent.is_some() {
            parse_error!(self.peek().line_number, "Function declarations cannot be nested inside other scopes");
        }
        self.consume();
        // consume the 'fn' keyword
        let mut return_type_token = Token::new(TokenType::Null, self.peek().line_number); // default return type is null/void
        if matches!(self.peek().token_type, TokenType::LessThan) {
            self.consume();
            // consume the less than sign
            return_type_token = self.peek().clone();
            if matches!(return_type_token.token_type, TokenType::F32 | TokenType::I32 | TokenType::Str | TokenType::Bool | TokenType::Null) {
                self.consume();
            } else {
                parse_error!(self.peek().line_number, "Expected return type declaration (currently supported: f32, i32, str, bool, null/void)");
            }
            if !matches!(self.peek().token_type, TokenType::GreaterThan) {
                parse_error!(self.peek().line_number, "Expected '>' after return type declaration");
            }
            self.consume();
            // consume the greater than sign
        }
        if !matches!(self.peek().token_type, TokenType::Equals) {
            parse_error!(self.peek().line_number, "Expected '=' after function declaration");
        }
        self.consume();
        // consume the equals sign
        if !matches!(self.peek().token_type, TokenType::LeftParen) {
            parse_error!(self.peek().line_number, "Expected '(' after function return type declaration");
        }
        let mut parameters: Vec<(String, ValueType)> = Vec::new();
        self.consume();
        // consume the left parenthesis
        if !matches!(self.peek().token_type, TokenType::RightParen) {
            loop {
                if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
                    parse_error!(self.peek().line_number, "Expected parameter name in function declaration");
                }
                let param_name = self.consume().get_value();
                if !matches!(self.peek().token_type, TokenType::Colon) {
                    parse_error!(self.peek().line_number, "Expected ':' after parameter name in function declaration");
                }
                self.consume(); // consume the colon
                let param_type_token = self.peek().clone();
                if matches!(param_type_token.token_type, TokenType::F32 | TokenType::I32 | TokenType::Str | TokenType::Bool | TokenType::Null) {
                    self.consume();
                } else {
                    parse_error!(self.peek().line_number, "Expected parameter type declaration (currently supported: f32, i32, str, bool, null)");
                }
                parameters.push((param_name, param_type_token.get_value_type_declaration()));
                if matches!(self.peek().token_type, TokenType::Comma) {
                    self.consume();
                } else {
                    break;
                }
            }
        }
        if !matches!(self.peek().token_type, TokenType::RightParen) {
            parse_error!(self.peek().line_number, "Expected ')' after function parameters");
        }
        if let Some(return_type) = self.lookup_function(&identifier, &self.scope) {
            parse_error!(self.peek().line_number, format!("Function '{}' is already defined with return type {:?}", identifier, return_type).as_str());
        }
        self.scope.functions.as_mut().unwrap().insert(identifier.clone(), return_type_token.get_value_type_declaration());
        self.consume();
        // consume the right parenthesis
        self.new_scope();
        for (param_name, param_type) in parameters.iter() {
            self.scope.identifiers.insert(param_name.clone(), param_type.clone());
        }
        let body = self.unscoped_block();
        self.pop_scope();
        if identifier == "main" {
            if return_type_token.get_value_type_declaration() != ValueType::Null {
                parse_error!(self.peek().line_number, "The 'main' function must have a return type of 'null/void'");
            }
            let main_node = StatementNode::Main {
                body: Box::new(body)
            };
            program.add_child(main_node);
        } else {
            let function_node = StatementNode::Function {
                identifier: identifier.clone(),
                return_type: return_type_token.get_value_type_declaration(),
                parameters,
                body: Box::new(body),
                is_static: !matches!(program, StatementNode::Class { .. })
            };
            program.add_child(function_node);
        }
    }
    
    fn loop_statement(&mut self, program: &mut StatementNode) {
        let loop_node = if matches!(self.peek().token_type, TokenType::LeftBrace) {
            StatementNode::Loop { range: None, condition: None, body: Box::new(self.block()) }
        } else if matches!(self.peek().token_type, TokenType::If) {
            self.consume();
            let condition = self.expression(0);
            if condition.value_type != ValueType::Boolean {
                parse_error!(self.peek().line_number, "Expected boolean expression in loop condition");
            }
            let body = self.block();
            StatementNode::Loop { range: None, condition: Some(condition), body: Box::new(body) }
        } else {
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
            let range = Some(ExpressionNode::new(ExpressionNodeType::Range { 
                identifier: identifier.clone(),
                range_type: expected_type.clone(), 
                start: Box::new(start), 
                end: Box::new(end) 
            }, expected_type.clone()));
            self.loop_counter += 1;
            self.new_scope();
            self.scope.identifiers.insert(identifier, expected_type);
            let body: Option<StatementNode> = Some(self.unscoped_block());
            self.pop_scope();
            StatementNode::Loop { range, condition: None, body: Box::new(body.expect("Expected loop body")) }
        };
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

    fn call_statement(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
            parse_error!(self.peek().line_number, "Expected identifier for function call");
        }
        let callee = ExpressionNode::new(ExpressionNodeType::Variable { identifier: self.peek().get_value() }, ValueType::Function);
        self.consume(); // consume the identifier
        let call_node = self.parse_call(callee);
        let call_statement_node = StatementNode::Call { call: call_node };
        program.add_child(call_statement_node);
    }

    fn class_declaration(&mut self, program: &mut StatementNode) {
        if !matches!(self.peek().token_type, TokenType::Identifier(_)) {
            parse_error!(self.peek().line_number, "Expected identifier for class name");
        }
        let class_name = self.consume().get_value();
        if self.scope.parent.is_some() {
            parse_error!(self.peek().line_number, "Class declarations cannot be nested inside other scopes");
        }
        if self.file_context.main_class.is_some() {
            parse_error!(self.peek().line_number, "Only one class declaration is allowed per file");
        }
        if !matches!(self.peek().token_type, TokenType::LeftBrace) {
            parse_error!(self.peek().line_number, format!("Expected '{{' after class name but got {:?}", self.peek()).as_str());
        }
        self.consume(); // consume the left brace
        self.new_scope();
        self.scope.functions = Some(HashMap::new());
        let mut class_node = StatementNode::Class { identifier: class_name, body: Box::new(StatementNode::Block { children: Vec::new() }) };
        while !matches!(self.peek().token_type, TokenType::RightBrace) {
            match self.peek().token_type {
                TokenType::Let => {
                    self.consume();
                    self.let_statement(&mut class_node);
                }
                _ => {
                    parse_error!(self.peek().line_number, format!("Unexpected token in class declaration: {:?}", self.peek()).as_str());
                }
            }
        }
        self.consume(); // consume the right brace
        self.file_context.main_class = Some(class_node);
        program.add_child(self.file_context.main_class.as_ref().unwrap().clone());
        self.pop_scope();
    }
}