use std::{fmt::{Formatter, Display}};


#[derive(Debug, Clone)]
pub enum Value {
    Null,
    Integer(i64),
    Double(f64),
    String(String),
    Boolean(bool)
}

impl Display for Value {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), std::fmt::Error> {
        match self {
            Value::Null => {
                write!(f, "null")
            },
            Value::Integer(value) => {
                write!(f, "{}", value)
            },
            Value::Double(value) => {
                write!(f, "{}", value)
            }
            Value::Boolean(value) => {
                write!(f, "{}", value)
            }
            Value::String(value) => {
                write!(f, "{}", value)
            }
        }
    }
}