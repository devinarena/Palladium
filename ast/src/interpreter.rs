
/// File: interpreter.rs
/// Author: Devin Arena
/// Date: 12/2/2022
/// Purpose: Handles walking the AST and executing the code

use crate::expression::expression::Expression;
use crate::expression::expression::Visitor;
use crate::value::Value;

pub struct Interpreter {
    pub ast: Expression,
}

impl Interpreter {
    pub fn new(ast: Expression) -> Interpreter {
        Interpreter {
            ast,
        }
    }
}

impl Visitor for Interpreter {
    fn visit_literal(&mut self, expr: Expression) -> Value {
        match expr {
            Expression::Literal(value) => {
                println!("Value: {:?}", value);
                value
            },
            _ => panic!("Unexpected expression type")
        }
    }
}