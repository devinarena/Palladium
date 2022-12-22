use super::expression::Expression;

#[derive(Debug, Clone)]
pub enum Statement {
    PrintStatement(Box<Expression>),
    ExpressionStatement(Box<Expression>),
}

impl VisitedStatement for Statement {
    fn visit(&mut self, visitor: &mut dyn StatementVisitor) {
        match self {
            Statement::PrintStatement(_) => {
                visitor.visit_print_statement(self.to_owned())
            }
            Statement::ExpressionStatement(_) => {
                visitor.visit_expression_statement(self.to_owned())
            }
        }
    }
}

pub trait VisitedStatement {
    fn visit(&mut self, visitor: &mut dyn StatementVisitor);
}

pub trait StatementVisitor {
    fn visit_print_statement(&mut self, print: Statement);
    fn visit_expression_statement(&mut self, exprStatement: Statement);
}