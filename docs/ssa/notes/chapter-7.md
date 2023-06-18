# Chapter 7. Introduction

* **Propagating Information Using SSA**:
  * The basic idea is to directly propagate information computed at the unique definition of a variable to all its uses.
* **Liveness**:
  * Liveness analysis can be accelerated without the requirement of any iteration to reach a fixed point: it only requires at most two passes over the CFG;
  * An extremely simple liveness check is possible.
* **Loop Tree and Induction Variables**:
  * Induction variable analysis is based on the detection of self references in the SSA representation and the extraction of the loop tree, which can be performed on the SSA graph as well.
* **Redundancy Elimination**:
  * A computation is **fully redundant** if it has also occurred earlier regardless of control flow, and a computation is **partially redundant** if it has occurred earlier only on certain paths;
  * SSAPRE algorithm and its variant, the speculative PRE, which (possibly speculatively) perform partial redundancy elimination (PRE);
  * redundancy elimination based on value analysis (GVN).
* **Alias Analysis**.