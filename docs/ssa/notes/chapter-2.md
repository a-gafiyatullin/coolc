# Chapter 2. Properties and Flavours

## 2.1 Def-Use and Use-Def Chains
**Def-use chains** - data structures that provide, for the single definition of a variable, the set of all its uses:
* useful in forward data-flow analysis.
* SSA combines the information as early as possible.

**Use-def chain** (consists of a single name) -  data structure that uniquely specifies the definition that reaches the use.
* SSA allows maintain it almost for free;
* simplifies backward propagation.

## 2.2 Minimality
SSA construction is a two-phase process:
1. placement of φ-functions;
2. renaming.

A definition *D* of variable *v reaches* a point *p* in the CFG if there exists a path from *D* to *p* that does not pass through another definition of *v*.

Code has the **single reaching-definition property** iff no program point can be reached by two definitions of the same variable.

**Minimality property** - the minimality of the number of inserted φ-functions if the single reaching-definition property is fullfiled.

Node is a *join node* if two paths converge at this node and no other CFG node:
* *S* - set of basic blocks;
* *J(S)* - set of join nodes of set S.

if *Dv* is the set of basic blocks containing definitions of *v*, then φ-functions should be instantiated in every basic block in *J(Dv)*.
* *J(S ∪ J(S)) = J(S)*.

## 2.3 Strict SSA Form and Dominance Property
A procedure is defined as **strict** if every variable is defined before it is used along every path from the entry to the exit point; otherwise, it is **non-strict**.

Under SSA strictness is equivalent to the **dominance property**.

Basic block *n1* **dominates** basic block *n2* if every path in the CFG from the entry point to *n2* includes *n1* (*n1 dom n2*):
* every basic block dominates itself;
* *n1* **strictly dominates** *n2* if *n1* dominates *n2* and *n1 != n2* (*n1 sdom n2*);
* **immediate dominator** of a node *N* is the unique node that strictly dominates *N* but does not strictly dominate any other node that strictly dominates *N* (*idom*).

**Minimal SSA form** - variant of SSA form that satisfies both the minimality and dominance properties.

A **dominator tree** is a tree where the children of each node are those nodes it immediately dominates:
* For each variable, its live range, i.e., the set of program points where it is live is a sub-tree of the dominator tree:
  * iteration-free algorithm to compute liveness sets;
  * efficient algorithms to test whether two variables **interfere** (live ranges intersect).

A variable is **alive** at a program point if there exists a definition of that variable that can reach this point, *and* if there exists a definition-free path to a use.

Intersection graph of live ranges is chordal:
* several NP-complete problems have linear-time solutions.

A traversal of the dominator tree, i.e., a “tree scan,” can colour all of the variables in the program, without requiring the explicit construction of an interference graph.

## 2.4 Pruned SSA Form
In **pruned SSA** form every **use point** for a given variable must be reached by exactly one definition, as opposed to all program points:
* fewer φ-functions;
* suppress the instantiation of a φ-function at the beginning of a basic block if *v* is not live at the entry point of that block:
  * use liveness analysis;
  * construct minimal SSA and use DCE.

## 2.5 Conventional and Transformed SSA Form
A **web** is the maximum unions of def-use chains that have either a use or a def in common.

*x* and *y* are **φ-related** to one another if they are referenced by the same φ-function:
* the transitive closure of this relation defines an equivalence relation that partitions the variables defined locally in the procedure into equivalence classes, the φ-webs.

**Conventional SSA** (C-SSA) - SSA form for which each φ-web is interference-free:
* destruction:
  * each φ-web can be replaced with a single variable;
  * all definitions and uses are renamed to use the new variable;
  * all φ-functions involving this equivalence class are removed.
* construction:
  * insert copy operations that dissociate interfering variables from the connecting φ-functions.

## 2.6 A Stronger Definition of Interference
Two live ranges intersect iff one contains the definition of the other.

If we can prove that *u* and *v* will always hold the same value at every place where both are live, then they do not actually interfere with one another.