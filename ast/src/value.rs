use std::fmt::{Display, self};


#[derive(Debug, Clone, Copy)]
pub enum Value {
    Null(),
    Integer(i32),
    Float(f32),
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
        }
    }
}

