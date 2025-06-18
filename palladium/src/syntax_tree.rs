use crate::token::Token;

pub trait Visit {
    fn visit(&self) -> Box<Vec<String>>;
    fn visit_literal(&self, _value_token: &Box<Token>) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_variable(&self, _identifier: &String) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_binary(&self, _left: &Box<ExpressionNode>, _operator: &Box<Token>, _right: &Box<ExpressionNode> , _parent_precedence: u8) -> Box<Vec<String>> { Box::new(Vec::new()) }

    fn visit_statement(&self, _statement: &StatementNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_main_statement(&self, _body: &StatementNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_let_statement(&self, _identifier: &String, _type_token: &Token, _expression: &ExpressionNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_output_statement(&self, _expression: &ExpressionNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_block_statement(&self, _children: &Vec<StatementNode>) -> Box<Vec<String>> { Box::new(Vec::new()) }
    fn visit_loop_statement(&self, _body: &StatementNode) -> Box<Vec<String>> { Box::new(Vec::new()) }
}

impl std::fmt::Debug for dyn Visit {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Visit trait object")
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum ValueType {
    Float,
    String,
    Boolean
}

#[derive(Debug)]
pub struct ExpressionNode {
    pub node_type: ExpressionNodeType,
    pub value_type: ValueType
}

// Expressions
#[derive(Debug)]
pub enum ExpressionNodeType {
    Literal {
        value_token: Box<Token>,
    },
    Variable {
        identifier: String,
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
    Main {
        body: Box<StatementNode>
    },
    Output {
        expression: ExpressionNode
    },
    Let {
        identifier: String,
        type_token: Token,
        expression: ExpressionNode
    },
    Block {
        children: Vec<StatementNode>
    },
    Loop {
        body: Box<StatementNode>
    },
    Break
}

#[derive(Debug)]
pub struct MainNode {
    pub body: StatementNode
}

impl ExpressionNode {
    pub fn new(node_type: ExpressionNodeType, value_type: ValueType) -> ExpressionNode {
        ExpressionNode { 
            node_type, 
            value_type
        }
    }
}

impl ExpressionNodeType {
    pub fn get_value(&self) -> String {
        match *self {
            ExpressionNodeType::Literal { ref value_token, .. } => value_token.get_value(),
            _  => panic!("Expected a literal expression node"),
        }
    }
}

impl StatementNode {
    pub fn add_child(&mut self, child: StatementNode) {
        if let StatementNode::Block { children } = self {
            children.push(child);
        } else if let StatementNode::Main { body } = self {
            body.add_child(child);
        } else {
            panic!("Cannot add child to non-block statement node");
        }
    }

    pub fn is_main(&self) -> bool {
        match *self {
            StatementNode::Main { .. } => true,
            _ => false
        }
    }

    pub fn has_body(&self) -> bool {
        match self {
            StatementNode::Main { body, .. } => body.has_body(),
            StatementNode::Block { .. } => true,
            _ => false
        }
    }
}