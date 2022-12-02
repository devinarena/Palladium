mod lexer;
mod parser;

mod token;
mod value;
mod expression {
    pub mod expression;
}

use std::env;


fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: {} <file>", args[0]);
        return;
    }

    let mut lexer = lexer::Lexer::new(args[1].clone());
    lexer.tokenize();
    dbg!(lexer);

    let mut parser = parser::Parser::new(lexer.tokens.clone());
    dbg!(parser.expression());
}
