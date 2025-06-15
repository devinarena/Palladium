use std::{fs::File, io::Write, path::Path, time::{Duration, Instant}};

use crate::{syntax_tree::{ExpressionNode, ExpressionNodeType, MainNode, StatementNode, Visit}, token::{Token, TokenType}};


pub struct Compiler {
    pub main_file_name: String,
    pub directory: String,
    pub compile_time: Duration
}

impl Compiler {
    pub fn new(main_file_name: String, directory: String) -> Compiler {
        Compiler { 
            main_file_name: main_file_name.replace(".java", ""),
            directory,
            compile_time: Duration::new(0, 0)
        }
    }

    pub fn compile(&mut self, main: MainNode) {
        let start_time = Instant::now();
        let mut program: String = String::new();
        program.push_str(format!("public class {} {{\n", self.main_file_name).as_str());
        program.push_str(main.visit().join("\n").as_str());
        program.push_str("\n}");
        let output_path = Path::new(&self.directory).join(format!("{}.java", self.main_file_name));
        let mut output = File::create(output_path).expect("Failed to create output file");
        output.write_all(program.as_bytes()).expect("Failed to write to output file");
        self.compile_time = start_time.elapsed();
    }
}

fn operator_precedence(operator: &Token) -> u8 {
    match operator.token_type {
        TokenType::Plus | TokenType::Minus => 1,
        TokenType::Star | TokenType::Slash => 2,
        _ => panic!("Expected an operator"),
    }
}

impl Visit for ExpressionNode {
    fn visit(&self) -> Box<Vec<String>> {
        if let ExpressionNodeType::Literal { ref value_token } = self.node_type {
            return self.visit_literal(value_token);
        } else if let ExpressionNodeType::Variable { ref identifier } = self.node_type {
            return self.visit_variable(identifier);
        } else if let ExpressionNodeType::Binary { ref left, ref operator, ref right } = self.node_type {
            return self.visit_binary(left, operator, right, 0);
        }
        panic!("Expected a literal or binary expression node");

    }

    fn visit_literal(&self, value_token: &Box<Token>) -> Box<Vec<String>> {
        let mut output = String::new();
        match value_token.token_type {
            TokenType::StringLiteral(ref value) => {
                output.push_str("\"");
                output.push_str(value);
                output.push_str("\"");
            },
            TokenType::Decimal(value) => {
                output.push_str(format!("{}f", value).as_str());
            },
            _ => panic!("Expected a literal token"),
        }
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }

    fn visit_variable(&self, _identifier: &String) -> Box<Vec<String>> {
        let mut output = String::new();
        match self.node_type {
            ExpressionNodeType::Variable { ref identifier } => {
                output.push_str(identifier);
            },
            _ => panic!("Expected a variable expression node"),
        }
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }

    fn visit_binary(&self, left: &Box<ExpressionNode>, operator: &Box<Token>, right: &Box<ExpressionNode>, parent_precedence: u8) -> Box<Vec<String>> {
        let prec = operator_precedence(operator);

        let lhs = if let ExpressionNodeType::Binary { ref left, ref operator, ref right } = left.node_type {
            self.visit_binary(left, operator, right, prec).join("")
        } else {
            left.visit().join("")
        };

        let rhs = if let ExpressionNodeType::Binary { ref left, ref operator, ref right } = right.node_type {
            self.visit_binary(left, operator, right, prec + 1).join("")
        } else {
            right.visit().join("")
        };

        let operator = match operator.token_type {
            TokenType::Plus => "+",
            TokenType::Minus => "-",
            TokenType::Star => "*",
            TokenType::Slash => "/",
            _ => panic!("Expected an operator"),
        };

        if prec < parent_precedence {
            Box::new(vec![format!("({} {} {})", lhs, operator, rhs)])
        } else {
            Box::new(vec![format!("{} {} {}", lhs, operator, rhs)])
        }
    }
}

impl Visit for StatementNode {
    fn visit(&self) -> Box<Vec<String>> {
        if let StatementNode::Output { ref expression } = *self {
            return self.visit_output_statement(expression);
        } else if let StatementNode::Let { ref identifier, ref type_token, ref expression } = *self {
            return self.visit_let_statement(identifier, type_token, expression);
        } else if let StatementNode::Block { ref children } = *self {
            return self.visit_block_statement(children);
        }
        panic!("Unexpected statement node {:?}", self);
    }

    fn visit_output_statement(&self, expression: &ExpressionNode) -> Box<Vec<String>> {
        let mut output = String::new();
        output.push_str("System.out.println(");
        output.push_str(&expression.visit().join(""));
        output.push_str(");");
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }

    fn visit_let_statement(&self, identifier: &String, type_token: &Token, expression: &ExpressionNode) -> Box<Vec<String>> {
        let mut output = String::new();
        match type_token.token_type {
            TokenType::F32 => output.push_str("float "),
            TokenType::Str => output.push_str("String "),
            _ => panic!("Expected a type token for let statement"),
        }
        output.push_str(identifier);
        output.push_str(" = ");
        output.push_str(&expression.visit().join(""));
        output.push_str(";");
        let mut v = Vec::new();
        v.push(output);
        return Box::new(v);
    }

    fn visit_block_statement(&self, _children: &Vec<StatementNode>) -> Box<Vec<String>> {
        let mut output = Vec::new();
        output.push("{".to_string());
        for child in _children {
            output.append(child.visit().as_mut());
        }
        output.push("}".to_string());
        Box::new(output)
    }
}

impl Visit for MainNode {
    fn visit(&self) -> Box<Vec<String>> {
        let mut output = Vec::new();
        output.push("public static void main(String[] args)".to_string());
        output.append(self.body.visit().as_mut());
        Box::new(output)
    }
}