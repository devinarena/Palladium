# Palladium
### An open source C-inspired C-powered programming language.

## !!NOTE!!
Palladium is both early in development and more of a hobby project. It may be used for small applications but is **NOT** production ready.

### What is Palladium?
Palladium is a bytecode VM interpreter written in C. Palladium uses a recursive descent Pratt Parser to generate VM opcodes to be interpreted. Palladium was built off of the single pass compiler built in the book CraftingInterpreters.

### Whats the purpose of Palladium?
Overall, the purpose of this project was to research and learn about all aspects of programming language development (lexing, parsing, interpreting, compiling, virtual machines, etc). I'm currently working on expanding Palladium to be a compiled language and developing the standard library.

### When did I start working on Palladium?
Palldium development began at the beginning of June 2022.

### What are some of the features of Palladium?
Palladium is a staticly-typed programming language with a syntax style similar to C with some changes. It features functions, pointers, different data types, modularity, etc.

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

### Some Examples

```

int main() {
    if (stl.argc < 3) {
        stl::write("Usage: pal file.pd [int] - Squares the number");
        ret 1;
    }

    int num = stl::atoi((stl.argv + 2)~);
    int result = num * num;
    stl::write("Your number squared is:");
    stl::write(stl::tostr(result));

    ret 0;
}

main();
```

This program takes a number in as the command line argument and prints its squared value.

### License
Palladium is open source under the MIT license. You can use it for whatever or contribute if you'd like. 
