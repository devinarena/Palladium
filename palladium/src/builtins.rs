use crate::syntax_tree::ValueType;


pub struct Builtin {
    pub name: String,
    pub args: Vec<String>,
    pub return_type: ValueType
}

impl Builtin {
    pub fn new(name: String, args: Vec<String>, return_type: ValueType) -> Self {
        Self { name, args, return_type }
    }
}