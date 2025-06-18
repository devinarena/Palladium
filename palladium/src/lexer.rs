use crate::token::{Token, TokenType};

pub struct Lexer {
    content: String,
    current: usize,
    output: Vec<Token>,
}

impl Lexer {

    pub fn new() -> Lexer {
        Lexer {
            content: String::new(),
            current: 0,
            output: Vec::new(),
        }
    }

    pub fn lex(&mut self, input: String) {
        self.output = Vec::new();
        self.content = input;
        self.current = 0;
        let mut line_number: u32 = 1;
        while self.peek() != '\0' {
            let mut current_char = self.peek();
            if current_char.is_numeric() {
                let mut current = String::new();
                while current_char.is_numeric() {
                    current.push(current_char);
                    current_char = self.next();
                }
                if current_char == '.' {
                    current.push(current_char);
                    current_char = self.next();
                    while current_char.is_numeric() {
                        current.push(current_char);
                        current_char = self.next();
                    }
                }
                let decimal_value: f64 = current.parse().unwrap();
                self.output.push(Token::new(TokenType::Decimal(decimal_value), line_number));
            } else if current_char.is_alphanumeric() || current_char == '_' {
                let mut current= String::new();
                while current_char.is_alphanumeric() || current_char == '_' {
                    current.push(current_char);
                    current_char = self.next();
                }
                match current.as_str() {
                    "output" => self.output.push(Token::new(TokenType::Output, line_number)),
                    "let" => self.output.push(Token::new(TokenType::Let, line_number)),
                    "f32" => self.output.push(Token::new(TokenType::F32, line_number)),
                    "str" => self.output.push(Token::new(TokenType::Str, line_number)),
                    "bool" => self.output.push(Token::new(TokenType::Bool, line_number)),
                    "true" => self.output.push(Token::new(TokenType::True, line_number)),
                    "false" => self.output.push(Token::new(TokenType::False, line_number)),
                    "and" => self.output.push(Token::new(TokenType::And, line_number)),
                    "or" => self.output.push(Token::new(TokenType::Or, line_number)),
                    "break" => self.output.push(Token::new(TokenType::Break, line_number)),
                    "loop" => self.output.push(Token::new(TokenType::Loop, line_number)),
                    "if" => self.output.push(Token::new(TokenType::If, line_number)),
                    "else" => self.output.push(Token::new(TokenType::Else, line_number)),
                    _ => self.output.push(Token::new(TokenType::Identifier(current), line_number)),
                }
            } else if current_char == '\"' {
                let mut current= String::new();
                current_char = self.next();
                while current_char != '\"' {
                    current.push(current_char);
                    current_char = self.next();
                }
                self.output.push(Token::new(TokenType::StringLiteral(current), line_number));
                self.next();
            } else if current_char == '(' {
                self.output.push(Token::new(TokenType::LeftParen, line_number));
                self.next();
            } else if current_char == ')' {
                self.output.push(Token::new(TokenType::RightParen, line_number));
                self.next();
            } else if current_char == '{' {
                self.output.push(Token::new(TokenType::LeftBrace, line_number));
                self.next();
            } else if current_char == '}' {
                self.output.push(Token::new(TokenType::RightBrace, line_number));
                self.next();
            } else if current_char == ':' {
                self.output.push(Token::new(TokenType::Colon, line_number));
                self.next();
            } else if current_char == '+' {
                self.output.push(Token::new(TokenType::Plus, line_number));
                self.next();
            } else if current_char == '-' {
                self.output.push(Token::new(TokenType::Minus, line_number));
                self.next();
            } else if current_char == '*' {
                self.output.push(Token::new(TokenType::Star, line_number));
                self.next();
            } else if current_char == '/' {
                self.output.push(Token::new(TokenType::Slash, line_number));
                self.next();
            } else if current_char == '=' {
                current_char = self.next();
                if current_char == '=' {
                    self.output.push(Token::new(TokenType::DoubleEquals, line_number));
                    self.next();
                } else {
                    self.output.push(Token::new(TokenType::Equals, line_number));
                }
            } else if current_char == '>' {
                current_char = self.next();
                if current_char == '=' {
                    self.output.push(Token::new(TokenType::GreaterEqualTo, line_number));
                    self.next();
                } else {
                    self.output.push(Token::new(TokenType::GreaterThan, line_number));
                }
            } else if current_char == '<' {
                current_char = self.next();
                if current_char == '=' {
                    self.output.push(Token::new(TokenType::LessEqualTo, line_number));
                    self.next();
                } else {
                    self.output.push(Token::new(TokenType::LessThan, line_number));
                }
            } else if current_char == '&' {
                current_char = self.next();
                if current_char == '&' {
                    self.output.push(Token::new(TokenType::And, line_number));
                    self.next();
                } else {
                    panic!("Unexpected character: {}", current_char);
                }
            } else if current_char == '|' {
                current_char = self.next();
                if current_char == '|' {
                    self.output.push(Token::new(TokenType::Or, line_number));
                    self.next();
                } else {
                    panic!("Unexpected character: {}", current_char);
                }
            } else if current_char == '\n' {
                line_number += 1;
                self.next();
            } else if current_char.is_whitespace() {
                self.next();
            } else {
                panic!("Unexpected character: {}", current_char);
            }
        }
        self.output.push(Token::new(TokenType::EndOfFile, line_number));
    }

    pub fn get_tokens(&self) -> &Vec<Token> {
        return &self.output;
    }

    fn peek(&self) -> char {
        if self.current >= self.content.len() {
            return '\0';
        }
        self.content.chars().nth(self.current).unwrap()
    }

    fn next(&mut self) -> char {
        self.current += 1;
        self.peek()
    }
}