use std::{env::args, path::Path, process::{exit, Command}, time::Instant};

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
    let (output_file, output_path) = get_output_file_name_and_path(&input_file);
    let input: String = std::fs::read_to_string(input_file).expect("Failed to read input file");
    let mut lexer = Lexer::new();
    lexer.lex(input);
    if args().any(|arg| arg == "--debug" || arg == "-d") {
        println!("TOKENS: {:?}", lexer.get_tokens());
    }
    let mut parser = Parser::new(output_file, lexer.get_tokens());
    let tree = parser.parse();
    if args().any(|arg| arg == "--debug" || arg == "-d") {
        println!("TREE: {:?}", tree);
    }
    let mut compiler = Compiler::new(parser.file_name.clone(), output_path);
    compiler.compile(tree);
    println!("Compilation to java finished in {}ms", parser.parse_time.as_millis() + compiler.compile_time.as_millis());
    let java_compile_start = Instant::now();
    if args().any(|arg| arg == "--run" || arg == "--compile") {
    let java_output_path = Path::new(&compiler.directory).join(format!("{}.java", compiler.main_file_name));
        let java_compilation_output = if cfg!(target_os = "windows") {
            Command::new("cmd").args(["/C", "javac", java_output_path.to_str().unwrap()]).output().expect("Failed to run shell command")
        } else {
            Command::new("sh").args(["-c", "javac", java_output_path.to_str().unwrap()]).output().expect("Failed to run shell command")
        };
        if !java_compilation_output.status.success() {
            eprintln!("!! Java compilation failed: !!");
            for  line in String::from_utf8_lossy(&java_compilation_output.stderr).lines() {
                eprintln!("\t{}", line);
            }
            exit(1);
        } else {
            println!("Java compilation finished in {}ms", java_compile_start.elapsed().as_millis());
            if args().any(|arg| arg == "--run") {
                let output = if cfg!(target_os = "windows") {
                    let mut args = vec!["/C", "java"];
                    if !compiler.directory.is_empty() {
                        args.push("-classpath");
                        args.push(&compiler.directory);
                    }
                    args.push(compiler.main_file_name.as_str());
                    Command::new("cmd").args(args).output().expect("Failed to run shell command")
                } else { 
                    let mut args = vec!["-c", "java"];
                    if !compiler.directory.is_empty() {
                        args.push("-classpath");
                        args.push(&compiler.directory);
                    }
                    args.push(compiler.main_file_name.as_str());
                    Command::new("sh").args(args).output().expect("Failed to run shell command")
                };
                println!("{}", String::from_utf8_lossy(&output.stdout));
            }
            exit(0);
        }
    }
}


fn get_output_file_name_and_path(input_file: &str) -> (String, String) {
    let path = Path::new(input_file);
    if !path.exists() {
        panic!("Input file does not exist: {}", input_file);
    }
    let mut output_file = path.file_stem().unwrap().to_str().unwrap().to_string().chars().filter(|c| c.is_alphanumeric() || *c == '_').collect::<String>();
    if let Some(first_char) = output_file.chars().next() {
        output_file.replace_range(0..1, &first_char.to_uppercase().to_string());
    }
    let output_path = path.parent().unwrap_or_else(|| Path::new(".")).to_str().unwrap().to_string();
    (output_file, output_path)
}