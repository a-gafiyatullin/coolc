# Chapter 10. Loop Tree and Induction Variables

## 10.1 Part of the CFG and Loop Tree Can Be Exposed Through the SSA

### 10.1.1 An SSA Representation Without the CFG

### 10.1.2 Natural Loop Structures on the SSA

* **Loop-φ nodes** “i = φ(x, j)” have an argument that contains a self-reference j and an invariant argument x;
* **Close-φ nodes** “k = $φ_{exit}(i)$” capture the last value of a name defined in a loop. Names defined in a loop can only be used within that loop or in the arguments of a close-φ node (which is “closing” the set of uses of the names defined in that loop).

### 10.1.3 Improving the SSA Pretty Printer for Loops

## 10.2 Analysis of Induction Variables

The purpose of the induction variable analysis is to provide a characterization of the sequences of values taken by a variable during the execution of a loop:
* an exact function of the canonical induction variable of the loop;
* an approximation of the values taken during the execution of the loop represented by values in an abstract domain.

A canonical induction variable with an initial value 0 and a stride 1 that would occur in the loop with label x will be represented by the chain of recurrences $\{0,+, 1\}_x$ .

### 10.2.1 Stride Detection

The first phase of the induction variable analysis is the detection of the strongly connected components of the SSA:
* traverse the use-def SSA chains and detect that some definitions are visited twice.

### 10.2.2 Translation to Chains of Recurrences

$\{\{0,+,1\}_x,+,2\}_y$ defines a multivariate chain of recurrences with a step of 1 in loop *x* and a step of 2 in loop *y*, where loop *y* is enclosed in loop *x*.

$\{3,+,\{8,+,5\}_x\}_x$ = $\{3,+,8,+,5\}_x$ represents a polynomial evolution of degree 2 in loop x.

The semantics of a chain of recurrences is defined using the binomial coefficient $\binom{n}{k} = \frac{n!}{k!(n-k)!}$, by the equation: $\{c_0,+,c_1,+,c_2,+,...,+, c_n\}_x(l_x^{\rightarrow})=\sum_{p=0}^n\binom{l_x}{p}$

with $l_x^{\rightarrow}$ the iteration domain vector (the iteration loop counters of all the loops in which the chain of recurrences variates), and $l_x$ the iteration counter of loop *x*.

Split the analysis into two phases, with a symbolic representation as a partial intermediate result:
1. First, the analysis leads to an expression where the step part “s is left in a symbolic form, i.e., $\{c_0,+,s\}_x$;
2. Then, by instantiating the step, i.e., s = $\{c_1,+,c_2\}_x$, the chain of recurrences is that of a higher-degree polynomial, i.e., $\{c_0,+,\{c_1,+,c_2\}_x\}_x$ = $\{c_0,+,c_1,+,c_2\}_x$.