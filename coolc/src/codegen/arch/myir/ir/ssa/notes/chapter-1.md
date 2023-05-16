# Chapter 1. Introduction

**Static Single Assignment form** - a naming convention for storage locations (variables) in low-level representations of computer programs.

## 1.1 Definition of SSA
A program is defined to be in SSA form if each variable is a target of exactly one assignment statement in the program text.

Property: **referential transparency**.

## 1.2 Informal Semantics of SSA
**Target** - the variable being defined;

**Source** - right-hand side of any
assignment statements;

**φ-function** - pseudo-assignment function
* The purpose of a φ-function is to merge values from different incoming
paths, at control-flow merge points;
* if there are multiple φ-functions at the head of a basic block, then these are executed in parallel.

## 1.3 Comparison with Classical Data-Flow Analysis

Key advantages of the SSA-based analysis:
* Data-flow information propagates directly from definition statements to uses, via the def-use links implicit in the SSA naming scheme;
* The results of the SSA data-flow analysis are more succinct.