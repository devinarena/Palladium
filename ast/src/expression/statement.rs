use crate::expression::expression::Expression;

pub trait Visited {
    fn visit(&self, visitor: &mut dyn Visitor) -> Value;
}

#[derive(Debug, Clone)]
pub enum Statement {
    ExpressionStatement(Expression),
}

impl Visited for Statement {
    fn visit(&self, visitor: &mut dyn Visitor) -> Value {
        match self {
            Statement::ExpressionStatement(expression) => visitor.visit_expression_statement(self),
        }
    }
}

pub trait StatementVisitor {
    fn visit_expression_statement(&mut self, expression_statement: &Statement) -> ();
}
