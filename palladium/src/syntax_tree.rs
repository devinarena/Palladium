use crate::token::Token;


pub trait Visit {
    fn visit(&self) -> Box<Vec<String>>;
    fn visit_literal(&self, _value_token: &Box<Token>) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_variable(&self, _identifier: &String) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_binary(&self, _left: &Box<ExpressionNode>, _operator: &Box<Token>, _right: &Box<ExpressionNode>) -> Box<Vec<String>> { Box::new(Vec::new()) }

    fn visit_statement(&self, _statement: &StatementNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_let_statement(&self, _identifier: &String, _type_token: &Token, _expression: &ExpressionNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_output_statement(&self, _expression: &ExpressionNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
}

impl std::fmt::Debug for dyn Visit {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Visit trait object")
    }
}

// Expressions
#[derive(Debug)]
pub enum ExpressionNode {
    Literal {
        value_token: Box<Token>
    },
    Variable {
        identifier: String
    },
    Binary {
        left: Box<ExpressionNode>,
        operator: Box<Token>,
        right: Box<ExpressionNode>
    }
}

// Statements 

#[derive(Debug)]
pub enum StatementNode {
    Output {
        expression: ExpressionNode
    },
    Let {
        identifier: String,
        type_token: Token,
        expression: ExpressionNode
    }
}

#[derive(Debug)]
pub struct MainNode {
    pub children: Vec<StatementNode>
}

impl ExpressionNode {
    pub fn get_value(&self) -> String {
        match *self {
            ExpressionNode::Literal { ref value_token } => value_token.get_value(),
            _  => panic!("Expected a literal expression node"),
        }
    }
}

impl StatementNode {}

impl MainNode {
    pub fn new() -> MainNode {
        MainNode {
            children: Vec::new()
        }
    }

    pub fn new_with_children(children: Vec<StatementNode>) -> MainNode {
        MainNode {
            children
        }
    }

    pub fn add_child(&mut self, child: StatementNode) {
        self.children.push(child);
    }
}