use crate::{expression::expression::{Expression, Visitor, Visited}, value::Value};



pub struct Interpreter {

}

impl Interpreter {
    pub fn new() -> Interpreter {
        Interpreter {

        }
    }

    pub fn interpret(&mut self, expression: &mut Expression) -> Value {
        expression.visit(self)
    }
}

impl Visitor for Interpreter {
    fn visit_literal(&mut self, literal: Expression) -> Value {
        match literal {
            Expression::Literal(value) => {
                value
            }
        }
    }
}