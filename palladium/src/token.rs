#[derive(Debug, Clone)]
pub enum Token {
    EndOfFile,
    // Literals
    Identifier(String),
    Integer(i64),
    Decimal(f64),
    StringLiteral(String),
    // Keywords
    Output,
    Let,
    F32,
    // Operators
    LeftParen,
    RightParen,
    Colon,
    Plus,
    Minus,
    Star,
    Slash,
    Equals
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
        match self {
            Token::Identifier(id) => id.clone(),
            Token::Integer(num) => num.to_string(),
            Token::Decimal(num) => num.to_string(),
            Token::StringLiteral(lit) => format!("\"{}\"", lit),
            _ => panic!("Token does not have a value")
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