# 12-01: Introduction to Code Generation
- The accumulator is kept in MIPS register <b>$a0</b>.
- The address of the next location on the stack is kept in MIPS register <b>$sp</b>:
    - The top of the stack is at address <b>$sp + 4</b>.
- <b>lw r1 offset(r2)</b>:
    - Load 32-bit word from address <b>r2 + offset</b> into <b>r1</b>
- <b>add r1 2r r3</b>:
    - <b>r1 <- r2 + r3</b>
- <b>sw r1 offset(r2)</b>:
     - Store 32-bit word in <b>r1</b> at address <b>r2 + offset</b> into
- <b>addiu r1 r2 i</b>:
    - <b>r1 <- r2 + i</b>
- <b>li r i</b>:
    - <b>r <- i</b>

# 12-02: Code Generation I
- <b>beq r1 r2 label</b>:
    - Branch to label if <b>r1 = r2</b>
- <b>b label</b>:
    - Unconditional jump to label

# 12-03: Code Generation II
- <b>jal label</b>:
    - Jump to label, save address of next instruction in <b>$ra</b>
- <b>jr r</b>:
    - Jump to address in register <b>r</b>

# 12-05: Temporaries
- Let <b>NT(e) = #</b> of temps needed to evaluate <b>e</b>;
- <b>NT(e1 + e2)</b>
    - Needs at least as many temps as <b>NT(e1)</b>;
    - Needs at least as many temps as <b>NT(e2) + 1</b>;
- Space used for temps in <b>e1</b> can be reused for temps in <b>e2</b>.
- Code generation must know how many temporaries are in use at each point
- Add a new argument to code generation
    - the position of the next available temporary
- The temporary is used like a small, fixed-size stack

# 12-06: Object Layout
- Objects are laid out in contiguous memory
- Each attribute stored at a fixed offset in the object
    - The attribute is in the same place in every object of that class
- When a method is invoked, the object is <b>self</b> and the fields are the object's attributes
- The offset for an attribute is the same in a class and all of its subclassses
- Every class has a fixed set of methods
    - Including inherited methods
- A <b>dispatch table</b> indexes these methods
    - An array of method entry points
    - A method <b>f</b> lives at a fixed offset in the dispatch table for a class and all of its subclasses

# 13-01: Semantics Overview
- <b>Operational semantics</b> describes program eveluation via execution rules on an abstract machine.
- <b>Denotational semantics</b>
    - Program's meaning is a mathematical function
- <b>Axiomatic semantics</b>
    - Program behaviour described via logical formulas

# 13-02: Operational Semantics
- A variable environment maps variables to locations:
    - Keeps track of which variables are in scope;
    - Tells up where those variables are