![quark](assets/Quark.png)

Quark is a small compiler for a subset of the C language, written in C++ and targeting LLVM IR.

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
| --no-compile      | Skip Codegeneration  | 
| -o                | Emit Object file      |

## Overview

This compiler implements a minimal but structured pipeline: a lexer tokenizes the source, a parser builds a strongly typed abstract syntax tree, semantic analysis performs scope resolution and type checking, code generation emits LLVM IR, and an optimizer applies LLVM passes on top of that IR.

## Currently Supported Language Features

Quark supports `int` and `char` as base types, along with the built-in `uint8_t` and `uint16_t` types, pointers, and user-defined arrays (including multidimensional arrays) and structs. On the function side, it handles function definitions and parameters, direct and nested function calls, and return statements with type validation.

Control flow is covered through if-else blocks, while loops, for loops, and break and continue statements. Variables can be declared and optionally initialized at the point of declaration, and are properly block-scoped with correct handling of nested scopes. Integer and character literals are also supported as expressions.

On the optimization front, the compiler applies a hand-written equivalent of LLVM's mem2reg pass, along with power reductions, simple algebraic transformations, dead instruction elimination, dead branch elimination, and constant propagation. An AST printer is also included for inspecting the parsed tree during development.

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
| =     | Assignment                | a = 5 |
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

This also includes pixelc, which compiles to WASM and serves as an alternative to JavaScript for image processing workflows such as grayscaling and color inversion. You can try it out at the [pixelc web demo](https://vedjain773.github.io/quark/examples/pixelc/web/index.html).
