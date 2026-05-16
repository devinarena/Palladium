use std::{collections::HashMap, fs::File, io::Write, path::Path, sync::Arc, time::{Duration, Instant}};

use crate::{builtins::Builtin, parser::FileContext, syntax_tree::{ExpressionNode, ExpressionNodeType, StatementNode, ValueType}, token::{Token, TokenType}};


pub struct Compiler {
    pub file_ctx: FileContext,
    pub directory: String,
    pub compile_time: Duration,
    builtins: Arc<HashMap<String, Builtin>>
}

impl Compiler {
    pub fn new(file_ctx: FileContext, directory: String, builtins: Arc<HashMap<String, Builtin>>) -> Compiler {
        Compiler { 
            file_ctx,
            directory,
            compile_time: Duration::new(0, 0),
            builtins
        }
    }

    pub fn compile(&mut self) {
        if self.file_ctx.body.is_none() {
            panic!("Expected node with body");
        }
        let start_time = Instant::now();
        let mut program: String = String::new();
        for import in &self.file_ctx.imports {
            program.push_str(format!("import {};\n", import).as_str());
        }
        program.push_str(format!("public class {} {{\n", self.file_ctx.file_name).as_str());

        if self.file_ctx.imports.contains(&"java.util.Scanner".to_string()) {
            program.push_str("private static Scanner __palladium_scanner__ = new Scanner(System.in);\n");
            program.push_str("private static String __palladium_input__(String prompt) { System.out.print(prompt); return __palladium_scanner__.nextLine(); }\n");
        }

        // Emit functions at class level and other statements inside `main`
        let root_stmt = self.file_ctx.body.as_ref().unwrap();
        if let StatementNode::Main { body } = root_stmt {
            if let StatementNode::Block { children } = &**body {
                // First emit any class-level functions
                for child in children {
                    if let StatementNode::Function { .. } = child {
                        let func_lines = self.emit_statement(child);
                        program.push_str(&func_lines.join("\n"));
                        program.push_str("\n");
                    }
                }

                // Then emit the main method with non-function children
                program.push_str("public static void main(String[] args)\n{");
                for child in children {
                    if let StatementNode::Function { .. } = child {
                        continue;
                    }
                    let mut lines = self.emit_statement(child);
                    // Append each statement line inside main
                    for line in lines.drain(..) {
                        program.push_str("\n");
                        program.push_str(&line);
                    }
                }
                if self.file_ctx.imports.contains(&"java.util.Scanner".to_string()) {
                    program.push_str("\n__palladium_scanner__.close();");
                }
                program.push_str("\n}");
            } else {
                // Fallback to previous behavior
                let body_lines = self.emit_statement(root_stmt);
                program.push_str(&body_lines.join("\n"));
            }
        } else {
            let body_lines = self.emit_statement(root_stmt);
            program.push_str(&body_lines.join("\n"));
        }
        program.push_str("\n}");

        let output_path = Path::new(&self.directory).join(format!("{}.java", self.file_ctx.file_name));
        let mut output = File::create(output_path).expect("Failed to create output file");
        output.write_all(program.as_bytes()).expect("Failed to write to output file");
        self.compile_time = start_time.elapsed();
    }

    fn emit_expression(&self, expr: &ExpressionNode) -> String {
        match &expr.node_type {
            ExpressionNodeType::Literal { value_token } => {
                match value_token.token_type {
                    TokenType::StringLiteral(ref value) => format!("\"{}\"", value),
                    TokenType::Decimal(value) => format!("{}f", value),
                    TokenType::Integer(value) => value.to_string(),
                    TokenType::True => "true".to_string(),
                    TokenType::False => "false".to_string(),
                    _ => panic!("Expected a literal token"),
                }
            }
            ExpressionNodeType::Variable { identifier } => identifier.clone(),
            ExpressionNodeType::Binary { left, operator, right } => self.emit_binary(left, operator, right, 0),
            ExpressionNodeType::FunctionCall { callee, arguments, .. } => {
                let identifier = if let ExpressionNodeType::Variable { identifier } = &callee.node_type {
                    identifier.clone()
                } else {
                    panic!("(compiler) Expected a variable expression node for function call callee");
                };
                let args_out: Vec<String> = arguments.iter().map(|a| self.emit_expression(a)).collect();
                // Special-case builtins that map to Java library calls or operators
                if let Some(b) = self.builtins.get(&identifier) {
                    match b.name.as_str() {
                        "log" => return format!("System.out.println({})", args_out.join(", ")),
                        "print" => return format!("System.out.print({})", args_out.join(", ")),
                        "input" => return format!("__palladium_input__({})", args_out.join(", ")),
                        "len" => {
                            // string length: (expr).length()
                            return format!("({}).length()", args_out.join(", "));
                        }
                        "to_string" => return format!("String.valueOf({})", args_out.join(", ")),
                        "parse_int" => return format!("Integer.parseInt({})", args_out.join(", ")),
                        "parse_float" => return format!("Float.parseFloat({})", args_out.join(", ")),
                        "abs" => return format!("(float) Math.abs({})", args_out.join(", ")),
                        "sqrt" => return format!("(float) Math.sqrt({})", args_out.join(", ")),
                        "pow" => return format!("(float) Math.pow({}, {})", args_out.get(0).unwrap_or(&"0".to_string()), args_out.get(1).unwrap_or(&"0".to_string())),
                        "random" => return format!("Math.random()"),
                        _ => {}
                    }
                }
                format!("{}({})", identifier, args_out.join(", "))
            }
            ExpressionNodeType::Range { .. } => panic!("emit_expression: Range should only appear in loop headers"),
        }
    }

    fn emit_binary(&self, left: &Box<ExpressionNode>, operator: &Box<Token>, right: &Box<ExpressionNode>, parent_precedence: u8) -> String {
        let prec = operator_precedence(operator);

        let lhs = if let ExpressionNodeType::Binary { ref left, ref operator, ref right } = left.node_type {
            self.emit_binary(left, operator, right, prec)
        } else {
            self.emit_expression(left)
        };

        let rhs = if let ExpressionNodeType::Binary { ref left, ref operator, ref right } = right.node_type {
            self.emit_binary(left, operator, right, prec + 1)
        } else {
            self.emit_expression(right)
        };

        let op_str = match operator.token_type {
            TokenType::Plus => "+",
            TokenType::Minus => "-",
            TokenType::Star => "*",
            TokenType::Slash => "/",
            TokenType::And => "&&",
            TokenType::Or => "||",
            TokenType::GreaterThan => ">",
            TokenType::LessThan => "<",
            TokenType::GreaterEqualTo => ">=",
            TokenType::LessEqualTo => "<=",
            TokenType::DoubleEquals => "==",
            _ => panic!("(compiler) Expected an operator"),
        };

        if prec < parent_precedence {
            format!("({} {} {})", lhs, op_str, rhs)
        } else {
            format!("{} {} {}", lhs, op_str, rhs)
        }
    }

    fn emit_statement(&self, stmt: &StatementNode) -> Vec<String> {
        match stmt {
            StatementNode::Main { body } => {
                let mut out = Vec::new();
                out.push("public static void main(String[] args)".to_string());
                out.push("{".to_string());
                let body_out = self.emit_statement(body);
                out.append(&mut body_out[1..body_out.len()-1].to_vec());
                if self.file_ctx.imports.contains(&"java.util.Scanner".to_string()) {
                    out.push("__palladium_scanner__.close();".to_string());
                }
                out.push("}".to_string());
                out
            }
            StatementNode::Let { identifier, type_token, expression } => {
                let mut output = String::new();
                match type_token.token_type {
                    TokenType::F32 => output.push_str("float "),
                    TokenType::I32 => output.push_str("int "),
                    TokenType::Str => output.push_str("String "),
                    TokenType::Bool => output.push_str("boolean "),
                    _ => panic!("(compiler) Expected a type token for let statement"),
                }
                output.push_str(identifier);
                output.push_str(" = ");
                output.push_str(&self.emit_expression(expression));
                output.push_str(";");
                vec![output]
            }
            StatementNode::Block { children } => {
                let mut out = Vec::new();
                out.push("{".to_string());
                for child in children {
                    out.append(&mut self.emit_statement(child));
                }
                out.push("}".to_string());
                out
            }
            StatementNode::Loop { range, condition, body } => {
                let mut out = Vec::new();
                if let Some(range_expr) = range {
                    if let ExpressionNodeType::Range { identifier, range_type, start, end } = &range_expr.node_type {
                        let header = match range_type {
                            ValueType::Float => format!("for (float {} = {}; {} <= {}; {} += 1.0f)", identifier, self.emit_expression(start), identifier, self.emit_expression(end), identifier),
                            ValueType::Integer => format!("for (int {} = {}; {} <= {}; {} += 1)", identifier, self.emit_expression(start), identifier, self.emit_expression(end), identifier),
                            _ => panic!("(compiler) Expected a float or integer range type"),
                        };
                        out.push(header);
                        out.append(&mut self.emit_statement(body));
                        return out;
                    } else {
                        panic!("Expected a range expression node for loop statement");
                    }
                } else if condition.is_some() {
                    out.push("while (".to_string());
                    out.push(self.emit_expression(condition.as_ref().unwrap()));
                    out.push(")".to_string());
                } else {
                    out.push("while (true)".to_string());
                }
                out.append(&mut self.emit_statement(body));
                out
            }
            StatementNode::If { condition, body, else_body } => {
                let mut out = Vec::new();
                out.push("if (".to_string());
                out.push(self.emit_expression(condition));
                out.push(")".to_string());
                out.append(&mut self.emit_statement(body));
                if let Some(else_b) = else_body {
                    out.push("else".to_string());
                    out.append(&mut self.emit_statement(else_b));
                }
                out
            }
            StatementNode::Assignment { identifier, expression } => {
                vec![format!("{} = {};", identifier, self.emit_expression(expression))]
            }
            StatementNode::Call { call } => {
                vec![format!("{};", self.emit_expression(call))]
            }
            StatementNode::Return { expression } => {
                vec![format!("return {};", self.emit_expression(expression))]
            }
            StatementNode::Break => vec!["break;".to_string()],
            StatementNode::Function { identifier, parameters, return_type, body } => {
                let mut out = Vec::new();
                let params_str = parameters.iter().map(|(name, ty)| {
                    let ty_str = match ty {
                        ValueType::Float => "float",
                        ValueType::Integer => "int",
                        ValueType::String => "String",
                        ValueType::Boolean => "boolean",
                        _ => panic!("(compiler) Unsupported parameter type in function declaration"),
                    };
                    format!("{} {}", ty_str, name)
                }).collect::<Vec<String>>().join(", ");
                let return_str = match return_type {
                    ValueType::Float => "float",
                    ValueType::Integer => "int",
                    ValueType::String => "String",
                    ValueType::Boolean => "boolean",
                    ValueType::Null => "void",
                    _ => panic!("(compiler) Unsupported return type in function declaration"),
                };
                out.push(format!("public static {} {}({})", return_str, identifier, params_str));
                out.append(&mut self.emit_statement(body));
                out
            }
        }
    }
}

fn operator_precedence(operator: &Token) -> u8 {
    match operator.token_type {
        TokenType::Or => 0,
        TokenType::And => 2,
        TokenType::GreaterThan | TokenType::LessThan | TokenType::GreaterEqualTo | TokenType::LessEqualTo | TokenType::DoubleEquals => 4,
        TokenType::Plus | TokenType::Minus => 6,
        TokenType::Star | TokenType::Slash => 8,
        _ => panic!("Expected an operator"),
    }
}
