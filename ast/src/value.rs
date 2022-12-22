use std::{fmt::{Formatter, Display}};


#[derive(Debug, Clone, Copy)]
pub enum Value {
    Integer(i32),
}

impl Display for Value {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), std::fmt::Error> {
        match self {
            Value::Integer(value) => {
                write!(f, "{}", value)
            }
        }
    }
}