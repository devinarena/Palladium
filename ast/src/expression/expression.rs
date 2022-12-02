use crate::value::Value;

#[derive(Debug)]
pub enum Expression {
    Literal(Value),
}

