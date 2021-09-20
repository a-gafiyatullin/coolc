# 07-01: Predictive Parsing Part 1
- Like recursive-descent but parser can "predict" which production to use:
    - By looking at the next few tokens (<b>lookahead</b>);
    - No backtracking.
- Predictive patsers accept <b>LL(k)</b> grammars(<b>L</b>eft-to-right <b>L</b>eft-most derivation <b>k</b> tokens lookahead).
- In recursive-descent,
    - At each step, many choices of production to use;
    - Backtracking used to undo bad choices.
- In <b>LL(1)</b>,
    - At each step, only one choice of production.
- But can require left-factoring to eliminate the common prefix of multiple productions for one non terminal.

# 07-01: Predictive Parsing Part 2
Algorithm to use parsing table:
- Method similar to recursive descent, except:
    - For the leftmost non-terminal <b>S</b>
    - We look at the next input token a
    - And choice the poduction shown at <b>[S, a]</b>.

- A stack records frontier of parse tree
    - Non-terminals that have yet to be expanded;
    - Terminals that have yet to matched against the input;
    - Top of stack = leftmost pending terminal or non-terminal.

- Reject on reaching error state;
- Accept on end of input & empty stack.
<b>
initialize stack = < S, $ > and next<br>
repeat<br>
&emsp;case stack of<br>
&emsp;&emsp; < X, rest > : If T[X, *next] = Y1...Yn<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;then stack <- < Y1...Yn rest >;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;else error();<br>
&emsp;&emsp; < t, rest > : If t == *next++<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;then stack <- < rest >;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;else error();<br>
&emsp;until stack == < >
</b>

# 07-02: First Sets
- Consider non-terminal <b>A</b>, production <b>A -> α</b>, & token <b>t</b>;
- <b>T[A, t] = α,</b> in two cases:
    - If <b>α ->* tβ</b>:
        - α can derive a <b>t</b> in the first position;
        - We say that <b>t ∈ First(α).</b>
    - if <b>A -> α</b> and <b>α ->* ε</b> and <b>S ->* βAtδ</b>:
        - Useful if stack has <b>A</b>, input is <b>t</b>, and <b>A</b> cannot derive <b>t</b>;
        - In this case only option is to get rid of <b>A</b> (by deriving <b>ε</b>)
            - Can work only if <b>t</b> can follow <b>A</b> in at least one derivation.
        - We say  <b>t ∈ Follow(A).</b>

Definition:<br>
<b>First(X) = {t | X ->* tα} ⋃ {ε | X ->* ε}</b>, <b>X</b> - arbitrary string.<br>
Algorithm sketch:<br>
1. <b>First(t) = {t}</b>, <b>t</b> - terminal;
2. <b>ε ∈ First(X)</b>:
    - if <b>X -> ε</b>;
    - if <b>X -> A1...An</b> and <b>ε ∈ First(Ai) for 1 <= i <= n</b>.
3.  <b>First(α) ⊆ First(X)</b> if <b>X -> A1..Anα</b> and <b>ε ∈ First(Ai) for 1 <= i <= n</b>.

# 07-03: Follow Sets
- Definition:<br>
<b>Follow(X)</b> = {t | S ->* βXtδ}.<br>
- Intuition:<br>
    - if <b>X -> AB</b> then <b>First(B) ⊆ Follow(A)</b> and <b>Follow(X) ⊆ Follow(B)</b>;<br>
    if <b>B ->* ε</b> then <b>Follow(X) ⊆ Follow(A)</b>.
    - if <b>S</b> is the start symbol then <b>$ ∈ Follow(S)</b>.

Algorithm sketch:
1. <b>$ ∈ Follow(S)</b>;
2. <b>First(β) - {ε} ⊆ Follow(X)</b>
    - For each prodiction <b>A -> αXβ</b>.
3. <b>Follow(A) ⊆ Follow(X)</b>
    - For each production <b>A -> αXβ</b> where <b>ε ∈ First(β)</b>.

# 07-04: LL1 Parsing Tables
- Construct a parsing table T for CGG G:
- For each production <b>A -> α</b> in G do:
    - For each terminal <b>t ∈ First(α)</b> do:
        - <b>T[A, t] = α</b>
    - If <b>ε ∈ First(α)</b>, for each <b>t ∈ Follow(A)</b> do:
        - <b>T[A, t] = α</b>
    - If <b>ε ∈ First(α)</b> and <b>$ ∈ Follow(A)</b> do:
        - <b>T[A, $] = α</b>
- If any entry in the parsing table is multiply defined then <b>G</b> is not <b>LL(1)</b>. A grammar is defenitely not <b>LL(1)</b> if it is not left factored, is left recursive and ambiguous.
- Most programming languages GFGs are not <b>LL(1)</b>.