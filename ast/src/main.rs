mod lexer;
mod parser;
mod interpreter;

mod token;
mod value;
mod expression {
    pub mod expression;
}

use std::{env};

use crate::expression::expression::Expression;
use crate::token::Token;
use crate::lexer::Lexer;
use crate::parser::Parser;
use crate::interpreter::Interpreter;


fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: {} <file>", args[0]);
        return;
    }

    let mut lexer: Lexer = Lexer::new(args[1].clone());
    let tokens: Vec<Token> = lexer.tokenize();
    dbg!(lexer);

    let mut parser: Parser = Parser::new(tokens.clone());
    let mut ast: Expression = parser.expression();
    dbg!(ast);

    let mut interpreter: Interpreter = Interpreter::new();
    let value = interpreter.interpret(&mut ast);
    dbg!(value);
}
