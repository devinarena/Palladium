use crate::value::Value;

#[derive(Debug, Copy, Clone)]
pub enum Expression {
    Literal(Value),
}

impl Visited for Expression {
    fn visit(&mut self, visitor: &mut dyn Visitor) -> Value {
        match self {
            Expression::Literal(_) => {
                visitor.visit_literal(*self)
            }
        }
    }
}

pub trait Visited {
    fn visit(&mut self, visitor: &mut dyn Visitor) -> Value;
}

pub trait Visitor {
    fn visit_literal(&mut self, literal: Expression) -> Value;
}