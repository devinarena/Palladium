use std::{env::args, process::{exit, Command}, time::Instant};

use parser::Parser;

use crate::{compiler::Compiler, lexer::Lexer};

pub mod lexer;
pub mod parser;
pub mod compiler;
pub mod token;
pub mod syntax_tree;

fn main() {
    if args().len() < 2 {
        panic!("Usage: {} <input_file>", args().next().unwrap());
    }
    let input_file: String = args().nth(1).unwrap();
    let input: String = std::fs::read_to_string(input_file).expect("Failed to read input file");
    let mut lexer = Lexer::new();
    lexer.lex(input);
    println!("{:?}", lexer.get_tokens());
    let mut parser = Parser::new(lexer.get_tokens());
    let tree = parser.parse();
    println!("{:?}", tree);
    let mut compiler = Compiler::new();
    compiler.compile(tree);
    println!("Compilation to java finished in {}ms", parser.parse_time.as_millis() + compiler.compile_time.as_millis());
    let java_compile_start = Instant::now();
    let java_compilation_output = if cfg!(target_os = "windows") {
        Command::new("cmd").args(["/C", "javac", "Program.java"]).output().expect("Failed to run shell command")
    } else {
        Command::new("sh").args(["-c", "javac Program.java"]).output().expect("Failed to run shell command")
    };
    if !java_compilation_output.status.success() {
        eprintln!("!! Java compilation failed:");
        for  line in String::from_utf8_lossy(&java_compilation_output.stderr).lines() {
            eprintln!("\t{}", line);
        }
        exit(1);
    } else {
        println!("Java compilation finished in {}ms", java_compile_start.elapsed().as_millis());
        if args().any(|arg| arg == "--run") {
            let output = if cfg!(target_os = "windows") {
                Command::new("cmd").args(["/C", "java", "Program"]).output().expect("Failed to run shell command")
            } else { 
                Command::new("sh").args(["-c", "java Program"]).output().expect("Failed to run shell command")
            };
            println!("{}", String::from_utf8_lossy(&output.stdout));
        }
        exit(0);
    }
}
