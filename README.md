# MamaLang Compiler (Flex + Bison)

This project builds a mini compiler for MamaLang and translates MamaLang source into C code.

## Features covered

- **Lexical analysis** using Flex — token definitions, invalid token handling, Bison integration
- **Syntax analysis** using Bison — well-defined grammar rules for all language constructs
- **Semantic analysis / Type checking**:
  - Type system with int, float, char, and string types
  - Implicit type promotion (char → int → float)
  - Type error detection (e.g., arithmetic on strings)
  - Implicit conversion warnings (e.g., float → int precision loss)
  - Duplicate declaration detection
  - Undeclared variable detection
- **Correctness and Execution Behavior**:
  - Variable declaration and assignment
  - Arithmetic and relational expression evaluation
  - `if/else` conditional statements (including nested)
  - `while` loop
  - Custom `for` loop with optional double update
  - User-defined functions with parameters and return values
  - Recursive function calls
  - Nested function calls in expressions
- **Advanced Features**:
  - Source-to-source translation to C (intermediate code generation)
  - Constant folding optimization for compile-time expressions
  - Infinite loop detection with runtime safety counter for double-update loops
- **Extra Features** (from proposal):
  - Bangla-style error messages
  - Nested control structures
  - Double-update `jotokkhon` loop
  - Missing `edike_ay_mama` custom error

## Language keywords

| Keyword | Meaning |
|---------|---------|
| `edike_ay_mama` | program start (main) |
| `mama_chole_ja` | program end |
| `mama_eita_hoilo` | variable declaration |
| `bol_mama` | print |
| `shon_mama` | input (scanf) |
| `jodi_mama` | if |
| `nahole_mama` | else |
| `mama_jotokkhon` | while loop |
| `jotokkhon` | for loop (with double update) |
| `mama_kaj` | function definition |
| `ferot_de_mama` | return from function |

## Type system

MamaLang infers types from literal values:

| Literal | Inferred Type | C Type |
|---------|---------------|--------|
| `42` | int | `long long` |
| `3.14` | float | `double` |
| `'A'` | char | `char` |
| `"hello"` | string | `const char*` |

**Implicit conversions:** char → int → float (automatic promotion). Narrowing conversions (float → int) produce warnings. String ↔ numeric conversions are type errors.

## Function syntax

```
mama_kaj function_name(param1, param2) {
    // body
    ferot_de_mama result
}
```

Functions can be called in expressions or as statements:
```
mama_eita_hoilo result = function_name(5, 10)
function_name(5, 10)
```

## Prerequisites (Windows recommended via MSYS2)

Install:

- `flex`
- `bison`
- `gcc`
- `make` (if using Makefile)

If you install WinFlexBison on Windows, the commands may be `win_flex` and `win_bison`; the provided `build.ps1` supports both command styles.

MSYS2 UCRT64 example:

```bash
pacman -Syu
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-gcc flex bison make
```

## Build and run (PowerShell)

From project root:

```powershell
./build.ps1
```

This will:

1. Generate parser/lexer C files
2. Build `build/mamalangc.exe` (the MamaLang compiler)
3. Compile `examples/sample.mama` -> `build/generated.c`
4. Build and run `build/program.exe`

Use a custom source file:

```powershell
./build.ps1 -InputFile examples/functions.mama
```

## Build and run (Makefile in MSYS2 shell)

```bash
make
make run-sample
```

## Direct manual pipeline

```bash
bison -d -o build/parser.tab.c src/parser.y
flex -o build/lex.yy.c src/lexer.l
gcc -Wall -Wextra -O2 -Ibuild build/parser.tab.c build/lex.yy.c -o build/mamalangc
./build/mamalangc examples/sample.mama build/generated.c
gcc -Wall -Wextra -O2 build/generated.c -o build/program
./build/program
```

## Example programs

| File | Demonstrates |
|------|-------------|
| `examples/sample.mama` | Full demo: variables, conditionals, loops, functions, types |
| `examples/functions.mama` | Function definition, calls, recursion, nesting |
| `examples/error_undeclared.mama` | Undeclared variable error |
| `examples/error_missing_start.mama` | Missing `edike_ay_mama` error |
| `examples/error_type.mama` | Type checking error |

## Error messages (Bangla style)

- `'Edike ay mama' likhte bhule geso!!` — missing program start keyword
- `Mama, ei variable ke ami chini na: x` — undeclared variable
- `Mama, ei function ke ami chini na: foo` — undeclared function
- `Mama, infinite loop dhora porche!` — runtime infinite loop detection

## Notes

- Statements are newline separated.
- Blocks use `{ ... }`.
- Input form is `shon_mama(x)`.
- Char literals are accepted and converted to integer values internally.
- Float literals (e.g., `3.14`) are supported.
- Functions are defined with `mama_kaj` and must appear before `mama_chole_ja`.
- All function parameters and return values default to `long long`.
