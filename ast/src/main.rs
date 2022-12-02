mod interpreter;
mod lexer;
mod parser;

mod token;
mod value;
mod expression {
    pub mod expression;
}

use std::env;

use crate::token::Token;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: {} <file>", args[0]);
        return;
    }

    let mut lexer: lexer::Lexer = lexer::Lexer::new(args[1].clone());
    let tokens: Vec<Token> = lexer.tokenize();
    dbg!(lexer);

    let mut parser: parser::Parser = parser::Parser::new(tokens.clone());
    let expr = parser.expression().clone();
    dbg!(expr);

    let interpreter: interpreter::Interpreter = interpreter::Interpreter::new(expr);
    let result = interpreter.interpret();
}
