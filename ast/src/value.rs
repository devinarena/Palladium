use std::fmt::{Display, self};


#[derive(Debug, Clone)]
pub enum Value {
    Null(),
    Integer(i32),
    Float(f32),
    String(String),
}

impl Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Value::Null() => write!(f, "null"),
            Value::Integer(value) => {
                write!(f, "{}", value)
            }
            Value::Float(value) => {
                write!(f, "{}", value)
            }
            Value::String(value) => {
                write!(f, "{}", value)
            }
        }
    }
}

