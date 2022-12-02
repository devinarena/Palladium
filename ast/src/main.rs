mod lexer;
mod parser;

mod token;
mod value;
mod expression {
    pub mod expression;
}

use std::{env};

use crate::token::Token;


fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: {} <file>", args[0]);
        return;
    }

    let lexer: &mut lexer::Lexer = &mut lexer::Lexer::new(args[1].clone());
    let tokens: Vec<Token> = lexer.tokenize();
    dbg!(lexer);

    let parser: &mut parser::Parser = &mut parser::Parser::new(tokens.clone());
    dbg!(parser.expression());

    
}
