# Compiler Project Document

## Compiler Project Rubrics

### Lexical Analysis using Flex

-   Token Definitions\
-   Handling Invalid Tokens\
-   Integration with Bison

### Syntax Analysis using Bison

-   Well-defined grammar rules aligned with project proposal\
-   Syntax Error Handling (Type checking, implicit type conversion,
    variable declaration)

### correctness and Execution Behavior

-   Variable declaration & assignment\
-   Expression evaluation\
-   Conditional statements\
-   Loops and functions

Example Scenario: 
✔ Program can declare 
variables, Assign values, 
Print variables 
✔ Loops execute correctly 
✔ Functions return correct 
results 
etc. 

### Advanced Features

-   Intermediate Code Generation\
-   Code optimization techniques

------------------------------------------------------------------------

## MamaLang Compiler Project Proposal

### 1. Introduction

MamaLang is a mini-compiler project using conversational Bangla-English
keywords.\
It demonstrates lexical, syntax, semantic analysis and source-to-source
translation to C.

### 2. Language Features

-   Program start/end statements\
-   Variable declaration without data types\
-   Input/output operations\
-   Conditional statements\
-   Looping constructs

### 3. Variable Declaration Rules

**Syntax:**\
`mama_eita_hoilo <variable> = <value>`

**Examples:**\
`mama_eita_hoilo a = 20`\
`mama_eita_hoilo b = 'a'`

### 4. Key Mappings & Syntax

#### Program

-   edike_ay_mama → main()\
-   mama_chole_ja → end

#### I/O

-   bol_mama → printf
-   shon_mama → scanf

#### Condition

-   jodi_mama → if\
-   nahole_mama → else

#### Loop

-   mama_jotokkhon → while
-   jotokkhon -> for

### 5. Sample Code

    edike_ay_mama
    mama_eita_hoilo a = 20
    mama_eita_hoilo b = 10

    jodi_mama (a > b) {
        bol_mama("A boro")
    }
    nahole_mama {
        bol_mama("B boro")
    }

    mama_jotokkhon (a > 0) {
        bol_mama(a)
        a = a - 1
    }

    mama_chole_ja

### 6. Implementation Plan

1.  Lexical Analysis\
2.  Syntax Analysis\
3.  Semantic Analysis\
4.  Translation to C\
5.  Execution

### 7. Conclusion

MamaLang simplifies programming with natural language while
demonstrating compiler design concepts.


### 8.Some extra features: 
1. Error message style: 
Some sample error messages are: 
“‘Edike ay mama’ likhte bhule geso!!” ( missing the starting keyword) 
“Mama, ei variable ke ami chini na” (unidentified variable)

2. Nested Control Structure: 
It allows:  
Jodi mama inside jodi mama 
Mama jotokkhon inside conditionals 

3. A new Loop: Double Updatation 
jotokkhon ( i = 0 ; i < n ; i + 2 , i - 1 ) 
Here after the first iteration, the i value will increase by 2 and after the 2nd 
iteration the i value will decrease by 1. If the double parameter ends up in 
an infinite loop, an error message will be shown.

### 9.NB:

1. mama jotokkhon() is equivalent to while() and jotokkhon() is equivalent to for()

------------------------------------------------------------------------

## Additional Features Implemented

### 1. Function Definition and Calling

**New Keywords:**
-   `mama_kaj` → function definition
-   `ferot_de_mama` → return from function

**Syntax:**
```
mama_kaj function_name(param1, param2) {
    // body
    ferot_de_mama result
}
```

**Capabilities:**
-   Define functions with any number of parameters
-   Return values using `ferot_de_mama`
-   Recursive function calls (e.g., factorial)
-   Nested function calls in expressions (e.g., `max(square(3), square(2))`)
-   Function argument count validation
-   Forward declarations auto-generated in output C code

**Example:**
```
mama_kaj factorial(n) {
    jodi_mama (n <= 1) {
        ferot_de_mama 1
    }
    ferot_de_mama n * factorial(n - 1)
}

mama_eita_hoilo result = factorial(5)
bol_mama(result)
```

### 2. Type Checking System

**Supported Types (inferred automatically from literals):**

| Literal | Inferred Type | C Type |
|---------|---------------|--------|
| `42` | int | `long long` |
| `3.14` | float | `double` |
| `'A'` | char | `char` |
| `"hello"` | string | `const char*` |

**Implicit Type Conversion Rules:**
-   char → int → float (silent promotion, no warning)
-   float → int triggers a warning: `Warning: implicit conversion from float to int (precision loss)`
-   string ↔ numeric is a **type error**: `Type error: cannot convert string to int`
-   Arithmetic on strings is a **type error**: `Type error: cannot perform '+' on string values`

**Type-Aware I/O:**
-   `bol_mama` automatically selects the correct printf format (`%lld`, `%f`, `%c`, `%s`)
-   `shon_mama` automatically selects the correct scanf format

**Example triggering type error:**
```
mama_eita_hoilo name = "hello"
mama_eita_hoilo x = name + 5     // Type error!
```

### 3. Custom Bangla Error Message for Missing Start Keyword

When the program is missing `edike_ay_mama`, the compiler shows:
```
'Edike ay mama' likhte bhule geso!!
```

When an unknown function is called:
```
Mama, ei function ke ami chini na: foo
```

### 4. Infinite Loop Detection for Double-Update For Loop

The `jotokkhon` loop with double update now includes a runtime safety counter.
If the loop exceeds 1,000,000 iterations (likely caused by opposing updates cancelling each other out), the program prints:
```
Mama, infinite loop dhora porche! Loop force-stop.
```
and breaks out of the loop automatically.

### 5. Float Literal Support

The lexer now recognizes floating-point numbers (e.g., `3.14`, `0.5`) as a distinct `FLOATLIT` token, enabling float arithmetic and type-aware code generation.

### 6. String Variable Support

String literals can now be assigned to variables:
```
mama_eita_hoilo name = "hello"
bol_mama(name)
```

### 7. Scoped Symbol Table

Variables declared inside function bodies are local to that function. The compiler uses a scope stack to manage variable visibility, preventing name collisions between function parameters and global variables.

------------------------------------------------------------------------

## Updated Key Mappings & Syntax (Full)

| Keyword | Meaning |
|---------|---------|
| `edike_ay_mama` | Program start (main) |
| `mama_chole_ja` | Program end |
| `mama_eita_hoilo` | Variable declaration |
| `bol_mama` | Print (printf) |
| `shon_mama` | Input (scanf) |
| `jodi_mama` | If |
| `nahole_mama` | Else |
| `mama_jotokkhon` | While loop |
| `jotokkhon` | For loop (with double update) |
| `mama_kaj` | Function definition |
| `ferot_de_mama` | Return from function |
