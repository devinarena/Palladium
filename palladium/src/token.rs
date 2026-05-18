use crate::syntax_tree::ValueType;

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub line_number: u32
}

impl Token {
    pub fn new(token_type: TokenType, line_number: u32) -> Token {
        Token { token_type, line_number }
    }
}

#[derive(Debug, Clone)]
pub enum TokenType {
    EndOfFile,
    // Literals
    Identifier(String),
    Integer(i64),
    Decimal(f64),
    StringLiteral(String),
    // Keywords
    Let,
    Fn,
    F32,
    I32,
    Str,
    Null,
    Bool,
    True,
    False,
    Loop,
    Break,
    Return,
    If,
    Else,
    As,
    // Operators
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Colon,
    Dot,
    DoubleDot,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    And,
    Or,
    Equals,
    DoubleEquals,
    GreaterThan,
    LessThan,
    GreaterEqualTo,
    LessEqualTo,
    Not,
    NotEqualTo
}

impl Token {
    // fn as_string_literal(&self) -> Option<&String> {
    //     if let Token::Identifier(val) = self {
    //         Some(val)
    //     } else {
    //         None
    //     }
    // }
    pub fn get_value(&self) -> String {
        match &self.token_type {
            TokenType::Identifier(id) => id.clone(),
            TokenType::Integer(num) => num.to_string(),
            TokenType::Decimal(num) => num.to_string(),
            TokenType::StringLiteral(lit) => format!("\"{}\"", lit),
            TokenType::True => "true".to_string(),
            TokenType::False => "false".to_string(),
            _ => panic!("Token does not have a value")
        }
    }

    pub fn get_value_type_declaration(&self) -> ValueType {
        match &self.token_type {
            TokenType::F32 => ValueType::Float,
            TokenType::Str => ValueType::String,
            TokenType::Bool => ValueType::Boolean,
            TokenType::I32 => ValueType::Integer,
            TokenType::Null => ValueType::Null,
            TokenType::Fn => ValueType::Function,
            _ => panic!("Token does not have a value type declaration")
        }
    }
}

// impl Display for Token {
//     fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
//         match self {
//             Token::EndOfFile => write!(f, "EndOfFile"),
//             Token::Identifier(id) => write!(f, "Identifier({})", id),
//             Token::Integer(num) => write!(f, "Integer({})", num),
//             Token::Decimal(num) => write!(f, "Decimal({})", num),
//             Token::StringLiteral(lit) => write!(f, "StringLiteral({})", lit),
//             Token::LeftParen => write!(f, "LeftParen"),
//             Token::RightParen => write!(f, "RightParen"),
//         }
//     }
// }