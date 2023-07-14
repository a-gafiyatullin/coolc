# Chapter 9. Liveness

A **loop nesting forest** is a data structure that represents the loops in a CFG and the containment relation between them.

A **reducible CFG** is one with edges that can be partitioned into two disjoint sets: forward edges, and back edges, such that:
* Forward edges form a **directed acyclic graph** with all nodes reachable from the entry node;
* For all back edges *(A, B)*, node *B* dominates node *A*.

if CFG is **reducible**:
* *v* is live at a program point *q* if and only if *v* is live at the entry *h* of the largest loop/basic block (highest node in the loop nesting forest) that contains *q* but not the definition of *v*;
* *v* is live at *h* if and only if there is a path in the forward-CFG from *h* to a use of *v* that does not contain the definition.

**Consequence**: algorithm that computes liveness sets within two passes:
1. Compute partial liveness sets by traversing the forward-CFG (DAG) backwards;
2. Refine the partial liveness sets and computes the final solution by propagating forwards along the loop nesting forest.

The set of program points where a variable is live (live range) can be computed using a backward traversal starting on its uses and stopping when its definition is reached (unique under SSA).

**Liveness check**:

Algorithm (not based on setting up and solving data-flow equations):
1. a pre-computation part:
   1. independent of variables;
   2. depends on the structure of the control-flow graph;
   3. pre-computed information remains valid when variables or their uses are added or removed.
2. an online part executed at each liveness query:
   1. uses the def-use chain of the variable in question;
   2. determines the answer essentially by testing membership in pre-computed sets of basic blocks.

## 9.1 Definitions

Liveness is a property that relates program points to sets of variables which are considered to be **live** at these program points.

For a CFG node *q*, representing an instruction or a basic block, a variable *v* is **live-in** at *q* if there is a path, not containing the definition of *v*, from *q* to a node where *v* is used (including q itself). It is **live-out** at *q* if it is live-in at some direct successor of *q*.

**Liveness analysis** - the computation of live-in and live-out sets at the entry and the exit of basic blocks.

**Live-range** of a variable specifies the set of program points where that variable is live.

**Definition 9.1** (*Liveness for φ-Function Operands—Multiplexing Mode*)

For a *φ-function* ${a_0 = φ(a_1, . . . , a_n)}$ in block ${B_0}$, where ${a_i}$ comes from block ${B_i}$:
* Its definition-operand is considered to be at the entry of ${B_0}$, in other words variable ${a_0}$ is live-in of ${B_0}$;
* Its use operands are at the exit of the corresponding direct predecessor basic blocks, in other words, variable ${a_i}$ is live-out of basic block ${B_i}$.