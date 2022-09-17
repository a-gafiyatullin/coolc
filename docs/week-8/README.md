# 14-01: Intermediate Code
<b>Intermediate language</b> = high-level assembly
- Uses register names, but has an unlimited number;
- Uses control structures like assembly language;
- Uses opcodes but some are higher level.

<b>Three-address code</b>:
- <b>x := y op z</b>
- <b>x := op y</b>
- <b>y</b> and <b>z</b> are registers or constants.

# 14-02: Optimization Overview
A <b>basic block</b> is a maximal sequence of instructions with:
- no labels(except at the first instruction);
- no jumps(except in the alst instruction).

A <b>control-flow graph</b> is a direct graph with
- Basic blocks as nodes
- An edge from block A to block B if the execution can pass from the last instruction in A to the first instruction in B.

Optimizations:
- <b>Local</b>: apply to a basic block in isolation;
- <b>Global</b>: apply to a CFG in isolation;
- <b>Inter-procedural</b>: apply accross method boundaries.

# 14-03: Local Optimization
- Constant folding;
- Eliminate unreachable basic blocks;
- Common subexpression elimination (using SSA);
- Copy/Constant propagation (using SSA).

Optimizing compilers repeat optimizations until no improvement is possible.

# 14-04: Peephole Optimization
<b>Peephole optimization</b> is effective for improving assembly code:
- The "peephole" is a short sequence of (usually contiguous) instructions;
- The optimizer replaces the sequence with another equivalent one(but faster).

# 15-02: Constant Propagation
To replace a use of <b>x</b> by a constant <b>k</b> we must know:
- On every path to use of <b>x</b>, the last assignment to <b>x</b> is <b>x := k</b>

Rules:
- If <b>C(pi, x, out) = T</b> for any <b>i</b>, then <b>C(s, x, in) = T</b>, pi - predecessors of <b>s</b>;
- If <b>C(pi, x, out) = c</b> & <b>C(pj, x, out) = d</b> & <b>d != c</b>, then <b>C(s, x, in) = T</b>;
- If <b>C(pi, x, out) = c or ⊥</b> for all <b>i</b>, then <b>C(s, x, in) = c</b>;
- If <b>C(pi, x, out) = ⊥</b> for all <b>i</b>, then <b>C(s, x, in) = ⊥</b>;
- <b>C(s, x, out) = ⊥</b> if <b>C(s, x, in) = ⊥</b>;
- <b>C(x := c, x, out) = c</b> if <b>c</b> is a constant;
- <b>C(x := f(...), x, out) = T</b>;
- <b>C(y := ..., x, out) = C(y := ..., x, in)</b> if <b>x != y</b>.

Algorithm:
- For every entry <b>s</b> to the program, set <b>C(s, x, in) = T</b>;
- Set <b>C(s, x, in) = C(s, x, out) = ⊥</b> everywhere else;
- Repeat until all points satisfy 1-8:
    - Pick <b>s</b> not satisfying 1-8 and update using the appropriate rule.

# 15-03: Analysis of Loops
⊥ is necessary for cycles in Constant Propagation.

# 15-04: Orderings
- <b>T</b> is the gratest value, <b>⊥</b> is the least;
    - All constants are in between and incompatible;
- Let <b>lub</b> be the least-upper bound in this ordering;
- Rules 1-4 can be written using lub:
    - <b>C(s, x, in) = lub{ C(p, x, out) | p is a predecessor of s }</b>

Constant Propagation algorithm is linear in program size.

# 15-05: Liveness Analysis
A variable <b>x</b> is live at statement <b>s</b> if:
- There exists a statement <b>s'</b> that uses <b>x</b>;
- There is a path from <b>s</b> to <b>s'</b>;
- That path has no intervening assignment to <b>x</b>.

Rules:
- <b>L(p, x, out) = v { L(s, x, in) | s a successor of p}</b>
- <b>L(s, x, in) = true</b> if <b>s</b> refers to <b>x</b> on the <b>rhs</b>;
- <b>L(x := e, x, in) = false</b> if <b>e</b> does not refer to <b>x</b>;
- <b>L(s, x, in) = L(s, x, out)</b> if <b>s</b> does not refer to <b>x</b>.

Algorithm:
- Let all <b>L(...) = false</b> initially;
- Repeat until all statements  <b>s</b> satisfy rules 1-4:
    - Pick <b>s</b> where one of 1-4 does not hold and update using the appropriate rule.

Constant Propagation is a <b>forward analysis</b>: information is pushed from inputs to outputs;

Liveness is a <b>backward analysis</b>: information is pushed from outputs back towards inputs.