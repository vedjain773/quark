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
| --no-compile      | Skip Codegeneration   |
| --target          | Specify triple target |
| -o                | Emit Object file      |

### Language Features

Quark supports `int` and `char` as base types, along with `uint8_t`/`uint16_t`, pointers, arrays (including multidimensional), and structs. Functions support parameters, direct and nested calls, and typed return statements.

Control flow includes if-else, while loops, for loops, and break/continue. Variables can be declared with optional initialization and are block-scoped, including nested scopes.

### Optimizations

Quark applies a hand-written equivalent of LLVM's `mem2reg` pass, along with:
- Power reductions
- Simple algebraic transformations
- Dead instruction elimination
- Dead branch elimination
- Constant propagation

An AST printer is also included for inspecting the parsed tree during development.

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
