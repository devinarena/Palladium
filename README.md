# Palladium
## An open source C-inspired C-powered programming language.
### What is Palladium?
Palladium is a hobby and toy programming language. Palladium is a bytecode VM interpreter written in C. Palladium uses a recursive descent Pratt Parser to generate VM opcodes to be interpreted.

### Whats the purpose of Palladium?
Overall, the purpose of this project was to research and learn about all aspects of programming language development (lexing, parsing, interpreting, compiling, virtual machines, etc...). Palladium essentially is a hobby project, but one I also hope to make useful. I'm currently working on expanding Palladium to be a compiled language and developing the standard library.

### When did I start working on Palladium?
Palldium development began at the beginning of June 2022.

### What are some of the features of Palladium?
Palladium is a staticly-typed programming language with a syntax style similar to C with some changes. It features functions, pointers, different data types, modularity (WIP), etc.

### Getting Started
In order to use Palladium for your own project, you must first install the prerequisites:
* A C compiler
* GNU make

In order to build Palladium, simply build from the source by running the makefile with
```bash
make
```

#### Your First Program
Palladium is very simple to get started with. Here's the classic "hello world" example:
file.pd:
```
print "Hello, world!"
```

To run the file:
```bash
palladium file.pd
```

And you should see the output as "Hello, world!".

### License
Palladium is open source under the MIT license. You can use it for whatever or contribute if you'd like. 
