use std::collections::HashMap;

use crate::value::Value;


pub struct Environment {
    values: HashMap<String, Value>,
    parent: Option<Box<Environment>>,
}

impl Environment {
    pub fn new() -> Self {
        Self {
            values: HashMap::new(),
            parent: None,
        }
    }

    pub fn new_with_parent(parent: Box<Environment>) -> Self {
        Self {
            values: HashMap::new(),
            parent: Some(parent),
        }
    }

    pub fn get(&self, name: &String) -> Option<Value> {
        match self.values.get(name) {
            Some(value) => Some(value.to_owned()),
            None => {
                match &self.parent {
                    Some(parent) => parent.get(name),
                    None => None,
                }
            }
        }
    }

    pub fn assign(&mut self, name: &String, value: &Value) {
        match self.values.get_mut(name) {
            Some(value) => {
                *value = value.to_owned();
            }
            None => {
                match &mut self.parent {
                    Some(parent) => parent.assign(name, value),
                    None => {
                        self.values.insert(name.to_string(), value.clone());
                    }
                }
            }
        }
    }
}