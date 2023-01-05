use crate::{value::Value, token::{TokenType}};

#[derive(Debug, Clone)]
pub enum Expression {
    Literal(Value),
    Unary(TokenType, Box<Expression>),
    Binary(TokenType, Box<Expression>, Box<Expression>),
    Grouping(Box<Expression>),
    Variable(String),
}

impl VisitedExpression for Expression {
    fn visit(&mut self, visitor: &mut dyn ExpressionVisitor) -> Value {
        match self {
            Expression::Literal(_) => {
                visitor.visit_literal(self.to_owned())
            }
            Expression::Unary(_, _) => {
                visitor.visit_unary(self.to_owned())
            }
            Expression::Binary(_, _, _) => {
                visitor.visit_binary(self.to_owned())
            }
            Expression::Grouping(_) => {
                visitor.visit_grouping(self.to_owned())
            }
            Expression::Variable(_) => {
                visitor.visit_variable(self.to_owned())
            }
        }
    }
}

pub trait VisitedExpression {
    fn visit(&mut self, visitor: &mut dyn ExpressionVisitor) -> Value;
}

pub trait ExpressionVisitor {
    fn visit_literal(&mut self, literal: Expression) -> Value;
    fn visit_unary(&mut self, unary: Expression) -> Value;
    fn visit_binary(&mut self, binary: Expression) -> Value;
    fn visit_grouping(&mut self, grouping: Expression) -> Value;
    fn visit_variable(&mut self, variable: Expression) -> Value;
}