use crate::{
    expression::{
        expression::{Expression, ExpressionVisitor, VisitedExpression},
        statement::{Statement, StatementVisitor, VisitedStatement},
    },
    token::TokenType,
    value::Value, environment::Environment,
};

pub struct Interpreter {
    environment: Environment
}

impl Interpreter {
    pub fn new() -> Interpreter {
        Interpreter {
            environment: Environment::new()
        }
    }

    pub fn interpret(&mut self, statements: Vec<Statement>) {
        for mut stmt in statements {
            stmt.visit(self);
        }
    }
}

impl ExpressionVisitor for Interpreter {
    fn visit_literal(&mut self, literal: Expression) -> Value {
        match literal {
            Expression::Literal(value) => value,
            _ => panic!("Unexpected expression type"),
        }
    }

    fn visit_unary(&mut self, unary: Expression) -> Value {
        match unary {
            Expression::Unary(op, mut expr) => match op {
                TokenType::MINUS => match expr.visit(self) {
                    Value::Integer(value) => Value::Integer(-value),
                    _ => panic!("Unexpected value type"),
                },
                _ => panic!("Unexpected token type"),
            },
            _ => panic!("Unexpected expression type"),
        }
    }

    fn visit_binary(&mut self, binary: Expression) -> Value {
        match binary {
            Expression::Binary(op, mut left, mut right) => match op {
                TokenType::PLUS => match left.visit(self) {
                    Value::Integer(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => Value::Integer(left_value + right_value),
                        Value::Double(right_value) => {
                            Value::Double(left_value as f64 + right_value)
                        }
                        _ => panic!("Unexpected value type"),
                    },
                    Value::Double(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => {
                            Value::Double(left_value + right_value as f64)
                        }
                        Value::Double(right_value) => Value::Double(left_value + right_value),
                        _ => panic!("Unexpected value type"),
                    },
                    _ => panic!("Unexpected value type"),
                },

                TokenType::MINUS => match left.visit(self) {
                    Value::Integer(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => Value::Integer(left_value - right_value),
                        Value::Double(right_value) => {
                            Value::Double(left_value as f64 - right_value)
                        }
                        _ => panic!("Unexpected value type"),
                    },
                    Value::Double(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => {
                            Value::Double(left_value - right_value as f64)
                        }
                        Value::Double(right_value) => Value::Double(left_value - right_value),
                        _ => panic!("Unexpected value type"),
                    },
                    _ => panic!("Unexpected value type"),
                },

                TokenType::STAR => match left.visit(self) {
                    Value::Integer(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => Value::Integer(left_value * right_value),
                        Value::Double(right_value) => {
                            Value::Double(left_value as f64 * right_value)
                        }
                        _ => panic!("Unexpected value type"),
                    },
                    Value::Double(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => {
                            Value::Double(left_value * right_value as f64)
                        }
                        Value::Double(right_value) => Value::Double(left_value * right_value),
                        _ => panic!("Unexpected value type"),
                    },
                    _ => panic!("Unexpected value type"),
                },

                TokenType::SLASH => match left.visit(self) {
                    Value::Integer(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => {
                            if right_value == 0 {
                                panic!("Division by zero")
                            }
                            Value::Integer(left_value / right_value)
                        }
                        Value::Double(right_value) => {
                            if right_value == 0.0 {
                                panic!("Division by zero")
                            }
                            Value::Double(left_value as f64 / right_value)
                        }
                        _ => panic!("Unexpected value type"),
                    },
                    Value::Double(left_value) => match right.visit(self) {
                        Value::Integer(right_value) => {
                            if right_value == 0 {
                                panic!("Division by zero")
                            }
                            Value::Double(left_value / right_value as f64)
                        }
                        Value::Double(right_value) => {
                            if right_value == 0.0 {
                                panic!("Division by zero")
                            }
                            Value::Double(left_value / right_value)
                        }
                        _ => panic!("Unexpected value type"),
                    },
                    _ => panic!("Unexpected value type"),
                },
                _ => panic!("Unexpected token type"),
            },
            _ => panic!("Unexpected expression type"),
        }
    }

    fn visit_grouping(&mut self, grouping: Expression) -> Value {
        match grouping {
            Expression::Grouping(mut expr) => expr.visit(self),
            _ => panic!("Unexpected expression type"),
        }
    }

    fn visit_variable(&mut self, variable: Expression) -> Value {
        match variable {
            Expression::Variable(name) => {
                self.environment.get(&name).unwrap()
            },
            _ => panic!("Unexpected expression type"),
        }
    }
}

impl StatementVisitor for Interpreter {
    fn visit_expression_statement(&mut self, mut expr_statement: Statement) {
        expr_statement.visit(self);
    }

    fn visit_print_statement(&mut self, print: Statement) {
        match print {
            Statement::PrintStatement(mut expr) => println!("{}", expr.visit(self)),
            _ => panic!("Unexpected statement type"),
        }
    }
}
