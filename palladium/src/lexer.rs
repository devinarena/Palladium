use crate::token::{Token};

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
                self.output.push(Token::Decimal(decimal_value));
            } else if current_char.is_alphanumeric() || current_char == '_' {
                let mut current= String::new();
                while current_char.is_alphanumeric() || current_char == '_' {
                    current.push(current_char);
                    current_char = self.next();
                }
                match current.as_str() {
                    "output" => self.output.push(Token::Output),
                    "let" => self.output.push(Token::Let),
                    "f32" => self.output.push(Token::F32),
                    _ => self.output.push(Token::Identifier(current)),
                }
            } else if current_char == '\"' {
                let mut current= String::new();
                current_char = self.next();
                while current_char != '\"' {
                    current.push(current_char);
                    current_char = self.next();
                }
                self.output.push(Token::StringLiteral(current));
                self.next();
            } else if current_char == '(' {
                self.output.push(Token::LeftParen);
                self.next();
            } else if current_char == ')' {
                self.output.push(Token::RightParen);
                self.next();
            } else if current_char == ':' {
                self.output.push(Token::Colon);
                self.next();
            } else if current_char == '+' {
                self.output.push(Token::Plus);
                self.next();
            } else if current_char == '-' {
                self.output.push(Token::Minus);
                self.next();
            } else if current_char == '*' {
                self.output.push(Token::Star);
                self.next();
            } else if current_char == '/' {
                self.output.push(Token::Slash);
                self.next();
            } else if current_char == '=' {
                self.output.push(Token::Equals);
                self.next();
            } else if current_char.is_whitespace() {
                self.next();
            } else {
                eprintln!("Unexpected character: {}", current_char);
                std::process::exit(1);
            }
        }
        self.output.push(Token::EndOfFile);
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