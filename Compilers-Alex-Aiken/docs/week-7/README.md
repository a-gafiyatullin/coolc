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
    - Branch to label if <b>r1 = 2</b>
- <b>b label</b>:
    - Unconditional jump to label

# 12-03: Code Generation II
- <b>jal label</b>:
    - Jump to label, save address of next instruction in <b>$ra</b>
- <b>jr r</b>:
    - Jump to address in register <b>r</b>