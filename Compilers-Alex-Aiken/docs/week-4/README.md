# 07-01: Predictive Parsing Part 1
- Like recursive-descent but parser can "predict" which production to use:
    - By looking at the next few tokens (<b>lookahead</b>);
    - No backtracking.
- Predictive patsers accept LL(k) grammars(<b>L</b>eft-to-right <b>L</b>eft-most derivation <b>k</b> tokens lookahead).
- In recursive-descent,
    - At each step, many choices of production to use;
    - Backtracking used to undo bad choices.
- In LL(1),
    - At each step, only one choice of production.
- But can require left-factoring to eliminate the common prefix of multiple productions for one non terminal.

# 07-01: Predictive Parsing Part 2
Algorithm to use parsing table:
- Method similar to recursive descent, except:
    - For the leftmost non-terminal S
    - We look at the next input token a
    - And choice the poduction shown at [S, a].

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
<b>First(X)</b> = {t | X ->* tα} ⋃ {ε | X ->* ε}, <b>X</b> - arbitrary string.<br>
Algorithm sketch:<br>
1. First(t) = {t}, t - terminal;
2. ε ∈ First(X):
    - if X -> ε;
    - if X -> A1...An and ε ∈ First(Ai) for 1 <= i <= n.
3.  First(α) ⊆ First(X) if X -> A1..Anα and ε ∈ First(Ai) for 1 <= i <= n.
