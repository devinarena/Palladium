use crate::value::Value;

#[derive(Debug, Copy, Clone)]
pub enum Expression {
    Literal(Value),
}

pub trait Visitor {
    fn visit_literal(&mut self, expr: Expression) -> Value;
}
