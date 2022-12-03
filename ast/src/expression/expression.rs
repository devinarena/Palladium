use crate::{value::Value, token::{TokenType}};

#[derive(Debug, Clone)]
pub enum Expression {
    Literal(Value),
    Grouping(Box<Expression>),
    Unary(TokenType, Box<Expression>),
    Binary(Box<Expression>, TokenType, Box<Expression>),
}

impl Visited for Expression {
    fn visit(&self, visitor: &mut dyn Visitor) -> Value {
        match self {
            Expression::Literal(_) => {
                visitor.visit_literal(self)
            },
            Expression::Grouping(_) => {
                visitor.visit_grouping(self)
            },
            Expression::Unary(_, _) => {
                visitor.visit_unary(self)
            }
            Expression::Binary(_, _, _) => {
                visitor.visit_binary(self)
            }
        }
    }
}

pub trait Visited {
    fn visit(&self, visitor: &mut dyn Visitor) -> Value;
}

pub trait Visitor {
    fn visit_literal(&mut self, literal: &Expression) -> Value;
    fn visit_grouping(&mut self, grouping: &Expression) -> Value;
    fn visit_unary(&mut self, unary: &Expression) -> Value;
    fn visit_binary(&mut self, binary: &Expression) -> Value;
}
