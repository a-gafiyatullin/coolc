# 10-01: Static vs. Dynamic Typing
The <b>dynamic type</b> of an object is the class <b>C</b> that is used in the <b>"new C"</b> expression that created it;

The <b>static type</b> of an expression captures all dynamic types the expression could have.

Soundness theorem: for all expressions <b>E</b>:
<b>dynmaic_type(E) = static_type(E)</b>

# 11-03: Activation Records
The information needed to manage one procedure activation is called an <b>activation record (AR)</b> or <b>frame</b>.

# 11-06: Stack Machines
- Only storage is a stack
- An instruction <b>r = F(a1...an):</b>
    - Pops <b>n</b> operands from the stack
    - Computes the operation <b>F</b> using the operands
    - Pusches the result <b>r</b> on the stack
- An <b>n-register stack machine</b>
    - Conceptually, keep the top <b>n</b> locations of the pure stack machine's stack in registers
- Consider a 1-register stack machine
    - The register is called the <b>accumulator</b>

Algorithm:
- Consider an expression <b>op(e1...en)</b>
    - Note <b>e1...en</b> are subexpressions

- For each <b>ei(0 < i < n)</b>
    - Compute <b>ei</b>
    - Push result on the stack

- Pop <b>n - 1</b> values from the stack, compute <b>op</b>

- Store result in the accumulator