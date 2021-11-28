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