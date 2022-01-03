# 09-02: Scope

- Matching identifier declaration with uses;
- The <b>scope</b> of an identifier is the potion of a program in which that idedntifier is accessible;
- <b>Static scope</b> depends only on the program text;
- <b>Dynamic scope</b> depends on execution of the program.

# 09-03: Symbol Tables
- Much of semantic analysis can be expressed as a recursive descent of an AST;
- When performing semantic analysis on a portion of the AST, we need to know which identifiers are defined;
- A <b>symbol table</b> is a data structure that tracks the current bindings of identifiers;
- Semantic analysis requires multiple passes.

# 09-04: Types
- The goal of type checking is to ensure that operations are used only with the correct types;
- Three kinds of languages:
    - <b>Statically typed</b>: All or almost all checking of types is done as part of compilation;
    - <b>Dynamically typed</b>: Almost all checking of types is done as part of program execution;
    - <b>Untyped</b>: No type checking.

# 09-05: Type Checking
- The appropriate formalism for type checking is <b>logical rules of inference</b>.

# 09-06: Type Environments
- A <b>type environment</b> gives types for free variables:
    - A type environment is a function from <b>ObjectIdentifiers</b> to <b>Types</b>;
    - A variable is free in an expression if it not defines within the expression.
