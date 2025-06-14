use std::{fs::File, io::Write, time::{Duration, Instant}};

use crate::{syntax_tree::{ExpressionNode, MainNode, StatementNode, Visit}, token::Token};


pub struct Compiler {
    pub compile_time: Duration
}

impl Compiler {
    pub fn new() -> Compiler {
        Compiler { 
            compile_time: Duration::new(0, 0)
        }
    }

    pub fn compile(&mut self, main: MainNode) {
        let start_time = Instant::now();
        let mut program: String = String::new();
        program.push_str("public class Program {\n");
        program.push_str(main.visit().join("\n").as_str());
        program.push_str("\n}");
        let mut output = File::create("Program.java").expect("Failed to create output file");
        output.write_all(program.as_bytes()).expect("Failed to write to output file");
        self.compile_time = start_time.elapsed();
    }
}

impl Visit for ExpressionNode {
    fn visit(&self) -> Box<Vec<String>> {
        if let ExpressionNode::Literal { ref value_token } = *self {
            return self.visit_literal(value_token);
        } else if let ExpressionNode::Variable { ref identifier } = *self {
            return self.visit_variable(identifier);
        } else if let ExpressionNode::Binary { ref left, ref operator, ref right } = *self {
            return self.visit_binary(left, operator, right);
        }
        panic!("Expected a literal or binary expression node");

    }

    fn visit_literal(&self, value_token: &Box<Token>) -> Box<Vec<String>> {
        let mut output = String::new();
        match **value_token {
            Token::StringLiteral(ref value) => {
                output.push_str("\"");
                output.push_str(value);
                output.push_str("\"");
            },
            Token::Decimal(value) => {
                output.push_str(&value.to_string());
            },
            _ => panic!("Expected a literal token"),
        }
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }

    fn visit_variable(&self, _identifier: &String) -> Box<Vec<String>> {
        let mut output = String::new();
        match *self {
            ExpressionNode::Variable { ref identifier } => {
                output.push_str(identifier);
            },
            _ => panic!("Expected a variable expression node"),
        }
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }

    fn visit_binary(&self, left: &Box<ExpressionNode>, operator: &Box<Token>, right: &Box<ExpressionNode>) -> Box<Vec<String>> {
        let mut output = String::new();
        output.push_str(&left.visit().join(""));
        output.push_str(" ");
        match operator.as_ref() {
            Token::Plus => output.push_str("+"),
            Token::Minus => output.push_str("-"),
            Token::Star => output.push_str("*"),
            Token::Slash => output.push_str("/"),
            _ => panic!("Expected an operator in expression node"),
        }
        output.push_str(" ");
        output.push_str(&right.visit().join(""));
        let mut v = Vec::new();
        v.push(output);
        Box::new(v)
    }
}

impl Visit for StatementNode {
    fn visit(&self) -> Box<Vec<String>> {
        if let StatementNode::Output { ref expression } = *self {
            return self.visit_output_statement(expression);
        } else if let StatementNode::Let { ref identifier, ref type_token, ref expression } = *self {
            return self.visit_let_statement(identifier, type_token, expression);
        }
        panic!("Expected an output statement node");
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
        match type_token {
            Token::F32 => output.push_str("float "),
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
}

impl Visit for MainNode {
    fn visit(&self) -> Box<Vec<String>> {
        let mut output = Vec::new();
        output.push("public static void main(String[] args) {".to_string());
        for child in self.children.iter() {
            output.append(&mut child.visit());
        }
        output.push("}".to_string());
        Box::new(output)
    }
}