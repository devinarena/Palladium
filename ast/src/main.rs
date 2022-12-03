mod interpreter;
mod lexer;
mod parser;

mod token;
mod value;
mod expression {
    pub mod expression;
}

use std::env;

use crate::expression::expression::Expression;
use crate::interpreter::Interpreter;
use crate::lexer::Lexer;
use crate::parser::Parser;
use crate::token::Token;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: {} <file>", args[0]);
        return;
    }

    let mut lexer: Lexer = Lexer::new(args[1].clone());
    let tokens: Vec<Token> = lexer.tokenize();
    dbg!(lexer);
    dbg!(tokens.clone());

    let mut parser: Parser = Parser::new(tokens.clone());
    let ast: Expression = parser.expression();
    dbg!(ast.clone());

    let mut interpreter: Interpreter = Interpreter::new();
    let value = interpreter.interpret(&ast);
    println!("{}", value);
}
