# TinyLib Lexer Syntax Implementation Guide

This guide describes the first syntax target for the TinyLib preprocessor
language. The language keeps C-like expressions and statements, but removes a
few C grammar ambiguities so the lexer and parser can stay simple.

## Goals

- Keep C-like control flow, operators, blocks, literals, comments, and
  statement syntax.
- Make type positions explicit with `: type`.
- Make function declarations explicit with `fn`.
- Split typed declaration from assignment with `:=` and `=`.
- Use postfix `@` for indexing and named field/property access: `arr@1`,
  `arr@foo`.
- Reserve prefix `@EXPR` and `@EXPR[TYPE]` for compile-time metaprogramming
  syntax.
- Lower to C as a first backend target.

## Non-Goals For The First Pass

- Full C compatibility.
- C declarator compatibility such as `int *(*f)(void)`.
- Type inference beyond local parser convenience.
- Overload resolution or templates.

## Surface Syntax

### Types

Type names appear after `:` in declaration positions. Prefix `@` metaprogramming
forms may still use `[type]` for compile-time type arguments.

```c
int
u64
*int
const *char
ArrInt
```

Recommended first-pass type grammar:

```text
type          := type_qual* type_atom
type_qual     := "const" | "volatile"
type_atom     := "*" type_atom | identifier
```

This grammar is intentionally smaller than C. Add arrays, slices, and function
pointer types later after the basic parser is stable.

### Variables And Assignment

Use `name: type` for declarations. Add `:= expr` when the declaration also
initializes the binding. Use `=` only for assigning to an existing binding.

```c
count: int;
p: *int := &count;
count = count + 1;
```

Lowering target:

```c
int count;
int *p = &count;
count = count + 1;
```

Rules:

- `name: type;` declares a new binding without an initializer.
- `name: type := expr;` declares a new binding and initializes it.
- `name = expr` assigns to an existing binding.
- Re-declaring the same name in the same scope is an error.
- `name := expr` is not part of the first pass; require `: type` so
  declarations stay syntactically obvious.
- Top-level declarations should use explicit forms such as `fn`, `struct`, or a
  typed binding syntax such as `name: type;`.

### Functions

Function declarations start with `fn`, then a name, parameters, return type, and
body.

```c
fn add(a: int, b: int): int {
    return a + b;
}

fn log(msg: *char): void {
    puts(msg);
}
```

Lowering target:

```c
int add(int a, int b) {
    return a + b;
}

void log(char *msg) {
    puts(msg);
}
```

Recommended first-pass grammar:

```text
function_decl := "fn" identifier "(" param_list? ")" ":" type block
param_list    := param ("," param)*
param         := identifier ":" type
```

### Access With Postfix `@`

Postfix `@` is an access operator.

```c
arr@1
arr@i
arr@foo
```

The parser should represent this as one AST node:

```text
IndexOrMemberAccess(base, key)
```

Lowering should depend on the key:

- Integer literal or expression key: `arr@1` -> `arr[1]`
- Identifier key: first pass can lower `arr@foo` -> `arr.foo`

This keeps the surface syntax uniform while allowing the semantic pass to
choose array indexing, pointer indexing, struct field access, or custom
container access later.

Recommended grammar:

```text
postfix_expr := primary_expr postfix_op*
postfix_op   := "@" access_key
access_key   := integer_literal | identifier | "(" expression ")"
```

Allowing `arr@(i + 1)` avoids ambiguity for complex index expressions.

### Compile-Time Syntax With Prefix `@`

Prefix `@` is reserved for compiler-owned syntax and future metaprogramming,
similar in spirit to C3 builtins. The lexer should still return `@` as a raw
character token. The parser decides whether it is prefix metasyntax or postfix
access from context.

```c
@size_of[int]
@type_name[T]
@align_of[*int]
@embed("shader.glsl")
```

Recommended first-pass interpretation:

- `@name` starts a compile-time expression or compiler builtin.
- `@name(args...)` passes normal expression arguments.
- `@name[type]` passes a compile-time type argument or requests a compile-time
  type value.
- `@name(args...)[type]` is reserved but should be rejected until a concrete
  use case exists.

Recommended grammar:

```text
meta_expr     := "@" identifier meta_args? meta_type_arg?
meta_args     := "(" arg_list? ")"
meta_type_arg := "[" type "]"
```

The bracket after a prefix meta expression is a type argument, not array access
and not a normal declaration type marker.

Parser disambiguation:

- In prefix position, `@` starts `meta_expr`.
- After a primary/postfix expression, `@` starts postfix access.
- `arr@foo[int]` is not valid first-pass syntax; write `arr@foo` or a prefix
  meta expression separately.

## Lexer Changes

Add or confirm token support for:

- Keywords: `fn`, `return`, `if`, `else`, `while`, `for`, `break`, `continue`,
  `const`, `volatile`
- Punctuation: `[`, `]`, `(`, `)`, `{`, `}`, `,`, `;`, `@`
- Existing C-like operators: `+`, `-`, `*`, `/`, `%`, `:=`, `=`, `==`, `!=`, `<`,
  `<=`, `>`, `>=`, `&&`, `||`, `!`, `&`, `|`, `^`, `->`, `.`
- Literals: identifiers, integer literals, float literals, string literals,
  char literals

For the current `TLPP_*` lexer, add explicit token IDs only for multi-character
operators and keywords. Single-character tokens can remain their ASCII value.

Suggested new token IDs:

```c
TLPP_KW_FN,
TLPP_KW_RETURN,
TLPP_KW_IF,
TLPP_KW_ELSE,
TLPP_KW_WHILE,
TLPP_KW_FOR,
TLPP_KW_BREAK,
TLPP_KW_CONTINUE,
TLPP_KW_CONST,
TLPP_KW_VOLATILE,
TLPP_DECLARE,
```

`TLPP_DECLARE` represents `:=`. `@`, `[`, and `]` can be returned as raw
character tokens.

## Parser Shape

Use a recursive descent parser. Keep these layers separate:

```text
program        -> declaration* EOF
declaration    -> function_decl | variable_decl | statement
statement      -> block | return_stmt | if_stmt | while_stmt | expr_stmt
expression     -> assignment
assignment     -> logical_or ("=" assignment)?
variable_decl  -> identifier ":" type (";" | ":=" expression ";")
logical_or     -> logical_and ("||" logical_and)*
logical_and    -> equality ("&&" equality)*
equality       -> comparison (("==" | "!=") comparison)*
comparison     -> term (("<" | "<=" | ">" | ">=") term)*
term           -> factor (("+" | "-") factor)*
factor         -> unary (("*" | "/" | "%") unary)*
unary          -> ("!" | "-" | "&" | "*") unary | postfix
postfix        -> primary ("@" access_key | "(" arg_list? ")")*
primary        -> literal | identifier | meta_expr | "(" expression ")"
meta_expr      -> "@" identifier meta_args? meta_type_arg?
meta_type_arg  -> "[" type "]"
```

Types should not be parsed through expression grammar. A `:` token in a
declaration position starts `parse_type()`. A `[` token in expression position
is indexing or a future literal; a `[` token after prefix `@name` is a meta type
argument.

## AST Nodes

Minimum AST:

```text
Program(decls)
FnDecl(name, params, return_type, body)
Param(name, type)
VarDecl(name, type, init)
Block(stmts)
Return(expr)
If(cond, then_block, else_block)
While(cond, body)
ExprStmt(expr)
Binary(op, lhs, rhs)
Unary(op, expr)
Call(callee, args)
Access(base, key)
MetaExpr(name, args, type_arg)
Assign(lhs, rhs)
Identifier(name)
Literal(value)
Type(name, qualifiers, pointer_depth)
```

## Lowering To C

Lower types by reversing the colon type syntax into C declarator form.

Examples:

```text
name: int          -> int name
name: *int         -> int *name
name: const *char  -> const char *name
```

For the first pass, reject complicated type forms with a clear parser error
instead of trying to mimic C declarators.

Lower access:

```text
arr@1       -> arr[1]
arr@i       -> arr.i      // until semantic typing exists
arr@(i + 1) -> arr[i + 1]
```

After semantic typing exists:

```text
array/slice + integer/expression key -> base[key]
struct + identifier key              -> base.key
pointer-to-struct + identifier key    -> base->key
custom container + key                -> generated helper call
```

Lower meta expressions through compiler-owned lowering rules. Do not lower them
as normal calls unless that is explicitly the selected backend behavior.

```text
@size_of[int]   -> sizeof(int)
@align_of[int]  -> _Alignof(int)
@type_name[T]   -> compiler-generated string or metadata symbol
```

## Error Rules

Prefer hard parser errors for ambiguous C-like constructs:

- Reject declarations without `: type`.
- Reject initialized declarations that use `=` instead of `:=`.
- Reject assignment with `=` when the name has not been declared in an enclosing
  scope.
- Reject declarations when the name already exists in the same scope.
- Reject `name := expr` until inference is deliberately designed.
- Reject C-style type names before identifiers in declaration positions.
- Reject C-style function declarations like `int f(void)`.
- Reject complex access keys without parentheses: `arr@i + 1` parses as
  `(arr@i) + 1`; require `arr@(i + 1)` for one index expression.
- Reject unknown prefix `@name` forms unless the compiler has registered that
  metasyntax.
- Reject postfix `@` followed immediately by `[type]`; `@EXPR[TYPE]` is only a
  prefix metaprogramming form.

## First Implementation Milestone

1. Add keyword recognition in `preprocessor/lexer.c`.
2. Add `@` and `:=` tests in lexer token snapshots.
3. Add parser structs for `Type`, `FnDecl`, `VarDecl`, `Access`, and
   `MetaExpr`.
4. Parse and lower these examples:

```c
fn add(a: int, b: int): int {
    return a + b;
}

fn main(): int {
    xs: *int := get_values();
    n: int;
    n = xs@1;
    return n;
}
```

Expected C output:

```c
int add(int a, int b) {
    return a + b;
}

int main(void) {
    int *xs = get_values();
    int n;
    n = xs[1];
    return n;
}
```

5. Add negative tests for C-style type names before identifiers and C-style
   function declarations.
