![quark](assets/Quark.png)

Quark is a compiler for a subset of the C language, written in C++ and targeting LLVM IR.

## Usage
Clone the repository
```bash
git clone https://github.com/vedjain773/quark.git && cd quark 
```

Build the project
```bash
cmake --build build
```

Compile source
```bash
./quark input.c -o output.o
```

Link with an existing C/C++ file
```bash
g++ link.cpp output.o -o output
```

Execute
```bash
./output
```
| Flag              | Description           |
|-------------------|-----------------------|
| --print-tokens    | Print tokens          |
| --print-ast       | Print AST             |
| --print-llvm      | Print LLVM IR to a file |
| --optimize        | Apply Optimizations   |
| --no-compile      | Skip Codegeneration   |
| --target          | Specify triple target |
| -o                | Set destination file name |

### Compiler Features

The compiler features a custom scanner responsible for tokenizing the source code into tokens and then makes use of a recursive descent parser to construct an abstract syntax tree from the token stream. The compiler also includes an semantic analyser which is responsible for assigning types to expressions and ensuring the semantic correctness of the source code.

Once the semantic correctness of the program is validated, the compiler makes use of the LLVM compiler infrastructure for code generation to LLVM IR. Using LLVM enables quark to compile C code to any target architecture LLVM supports (a model which i'm a huge fan of!) 

Although LLVM also unlocks the door to the world-class optimizations it brings to the table, the compiler uses custom handwritten optimization passes (using the LLVM pass manager) to optimize the generated IR.

Also check out [Gluon](https://github.com/vedjain773/gluon)!

### Optimizations

Quark applies a hand-written equivalent of LLVM's `mem2reg` pass, along with:
- Power reductions
- Simple algebraic transformations
- Dead instruction elimination
- Dead branch elimination
- Constant propagation

An AST printer is also included for inspecting the parsed tree during development.

## Supported language features
- Integer types and expressions
- Local and global variables
- Pointers
- Arrays (including multi-dimensional arrays)
- Control flow through if-else blocks, while and for loops
- Blocks and nested scopes
- Break and continue statements
- Comparison and arithmetic operators
- Structs

## Supported Operators

### Arithmetic Operators
| Operator  | Description   | Example |
|-----------|---------------|---------|
| +         | Addition      | a + b |
| -         | Subtraction   | a - b |
| *         | Multiplication| a * b |
| /         | Division      | a / b |
| %         | Modulus       | a % b |

### Comparison Operators
| Operator  | Description       | Example |
|-----------|-------------------|---------|
| ==        | Equal to          | a == b  |
| !=        | Not equal to      | a != b  |
| >         | Greater than      | a > b   |
| <         | Less than         | a < b   |
| >=        | Greater or equal  | a >= b  |
| <=        | Less or equal     | a <= b  |

### Logical Operators
| Operator | Description | Example |
|---------|-------------|---------|
| &&    | Logical AND   | a && b |
| \|\|  | Logical OR    | a \|\| b |
| !     | Logical NOT   | !a |

### Pointer Operators
| Operator | Description | Example |
|---------|-------------|---------|
| *     | Dereference           | *ptr |
| []    | Access array element  | arr[i] |
| &     | AddressOf             | &x |

### Member Access Operators
| Operator | Description | Example |
|---------|-------------|---------|
| .     | Dot   | [struct].member |
| ->    | Arrow | [struct*]->member |

### Assignment Operators
| Operator | Description | Example |
|---------|-------------|---------|
| =     | Assignment                | a = 5  |
| +=    | Addition assignment       | a += 5 |
| -=    | Subtraction assignment    | a -= 5 |
| *=    | Multiplication assignment | a *= 5 |
| /=    | Division assignment       | a /= 5 |
| %=    | Modulus assignment        | a %= 5 |

### Miscellaneous Operators
| Operator | Description | Example |
|----------|-------------|---------|
| sizeof   | Size-of     | sizeof(i) or sizeof(int*)|

## Examples

Check out the [examples folder](examples) to see sample programs compiled by Quark.

- **Grayscale Filter** — [demo](https://vedjain773.github.io/quark/examples/pixelc/web/index.html)
- **Inversion Filter** — [demo](https://vedjain773.github.io/quark/examples/pixelc/web/neg.html)
- **Conway's Game of Life** — [demo](https://vedjain773.github.io/quark/examples/gol/web/index.html)
