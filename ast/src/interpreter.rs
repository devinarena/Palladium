use crate::{expression::expression::{Expression, Visitor, Visited}, value::{Value}, token::{TokenType}};



pub struct Interpreter {

}

impl Interpreter {
    pub fn new() -> Interpreter {
        Interpreter {

        }
    }

    fn runtime_error(&self, message: &str) {
        panic!("Runtime error: {}", message);
    }

    pub fn interpret(&mut self, expression: &Expression) -> Value {
        expression.visit(self)
    }
}

impl Visitor for Interpreter {
    fn visit_literal(&mut self, literal: &Expression) -> Value {
        match literal {
            Expression::Literal(value) => {
                *value
            }
            _ => {
                panic!("Expected literal");
            }
        }
    }

    fn visit_unary(&mut self, unary: &Expression) -> Value {
        match unary {
            Expression::Unary(token_type, expression) => {
                let value = expression.clone().visit(self);
                match token_type {
                    TokenType::MINUS => {
                        match value {
                            Value::Integer(integer) => {
                                Value::Integer(-integer)
                            }
                            Value::Float(float) => {
                                Value::Float(-float)
                            }
                            _ => {
                                self.runtime_error("Expected integer");
                                Value::Integer(0)
                            }
                        }
                    }
                    _ => {
                        self.runtime_error("Expected minus");
                        Value::Integer(0)
                    }
                }
            }
            _ => {
                panic!("Expected unary");
            }
        }
    }

    fn visit_binary(&mut self, binary: &Expression) -> Value {
        match binary {
            Expression::Binary(left, token_type, right) => {
                let left_value = left.clone().visit(self);
                let right_value = right.clone().visit(self);

                match token_type {
                    TokenType::PLUS => {
                        match left_value {
                            Value::Integer(left_integer) => {
                                match right_value {
                                    Value::Integer(right_integer) => {
                                        Value::Integer(left_integer + right_integer)
                                    }
                                    Value::Float(right_float) => {
                                        Value::Float(left_integer as f32 + right_float)
                                    }
                                    _ => {
                                        self.runtime_error("Operation undefined for types");
                                        Value::Integer(0)
                                    }
                                }
                            }
                            Value::Float(left_float) => {
                                match right_value {
                                    Value::Integer(right_integer) => {
                                        Value::Float(left_float + right_integer as f32)
                                    }
                                    Value::Float(right_float) => {
                                        Value::Float(left_float + right_float)
                                    }
                                    _ => {
                                        self.runtime_error("Operation undefined for types");
                                        Value::Integer(0)
                                    }
                                }
                            }
                            _ => {
                                self.runtime_error("Operation undefined for types");
                                Value::Integer(0)
                            }
                        }
                    },
                    TokenType::MINUS => {
                        match left_value {
                            Value::Integer(left_integer) => {
                                match right_value {
                                    Value::Integer(right_integer) => {
                                        Value::Integer(left_integer - right_integer)
                                    }
                                    Value::Float(right_float) => {
                                        Value::Float(left_integer as f32 - right_float)
                                    }
                                    _ => {
                                        self.runtime_error("Operation undefined for types");
                                        Value::Integer(0)
                                    }
                                }
                            }
                            Value::Float(left_float) => {
                                match right_value {
                                    Value::Integer(right_integer) => {
                                        Value::Float(left_float - right_integer as f32)
                                    }
                                    Value::Float(right_float) => {
                                        Value::Float(left_float - right_float)
                                    }
                                    _ => {
                                        self.runtime_error("Operation undefined for types");
                                        Value::Integer(0)
                                    }
                                }
                            }
                            _ => {
                                self.runtime_error("Operation undefined for types");
                                Value::Integer(0)
                            }
                        }
                    },
                    _ => panic!("Expected plus or minus")
                }
            }
            _ => {
                panic!("Expected binary");
            }
        }
    }
}