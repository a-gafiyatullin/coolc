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

# 07-05: Bottom-Up Parsing
- Bottom-up parsing is more general than (deterministic) top-down parsing
    - And just as efficient;
    - Builds on ideas in top-down parsing.
- Bottom-up is the preffered method;
- Bottom-up parsers don't need left-factored grammars;
- Bottom up parsing <b>reduces</b> a string to the start symbol by inverting productions;
- The productions, read backwards, trace a rightmost derivation;

- A bootom-up parser traces a rightmost derivation in reverse;

# 07-06: Shift-Reduce Parsing
- Let <b>αβω</b> be a step of a bottom-up parse;
- Assume the next reduction is by <b>X -> β</b>;
- Then <b>ω</b> is a string of terminals.

Idea: Slpit string into two substrings
- Right substring is as yet unexamined by parsing;
- Left substring has terminals and non-terminals;
- The dividing point is marked by a |.

<b>Shift</b>: Move | one place to the right
- Shift a terminal to the left string (read a terminal).

<b>Reduce</b>: Apply an inverse production at the right end of the left string.
- If <b>A -> xy</b> is a production, then <b>Cbxy | ijk -> CbA | ijk</b>.

Left string can be implemented by a stack
- Top of the stack is the |.

Shift pushes a terminal on the stack

Reduce
- pops symbols off of the stack;
- pushes a non-terminal on the stack.

In a given state, more than one action (shift or reduce) may lead to a valid parse.

If it is legal to shift or reduce, there is a <b>shift-reduce</b> conflict.

If it legal to reduce by two different productions, there is a <b>reduce-reduce</b> conflict.

# 08-01: Handles
<b>Intuition</b>: Want to reduce only if the result can still be reduced to the start symbol.

Assume a rightmost derivation <b>S ->* αXω -> αβω</b>.

Then <b>αβ</b> is a <b>handle</b> of <b>αβω</b>.

Handles formalize the intuition
- A handle is a reduction that also allows further reductions back to the start symbol.

We only want to reduce at handles.

In shift-reduce parsing, handles appear only at the top of the stack, never inside.

Handles are never to the left of the rightmost non-terminal
- Therefore, shift-reduce moves are sufficient; the | need never move left.

Bottom-up parsing algorithms are based on recognizing handles.

# 08-02: Recognizing Handles
<b>α</b> is a <b>viable prefix</b> if there is an <b>ω</b> such that <b>α | ω</b> is a state of a shift-reduce parser.
- A viable prefix does not extend past the right end of the handle;
- It's a viable prefix because it is a prefix of the handle;
- As long as a parser has viable prefixes on the stack no parsing error has been detected.

For any grammar, the set of viable prefixes is a regular language.

An <b>item</b> is a production with a <b>"."</b> somewhere on the rhs.

The only item for <b>X -> ε</b> is <b>X -> .</b>

Items are ofter called <b>LR(0) items</b>.

The stack may have many prefixes of rhs's:<br>
<b>Prefix(1) Prefix(2) ... Prefix(n-1) Prefix(n)</b>

Let <b>Prefix(i)</b> be a prefix of rhs of <b>X(i) -> α(i)</b>:
- <b>Prefix(i)</b> will eventually reduce to <b>X(i)</b>;
- The missing part of <b>α(i-1)</b> start with <b>X(i)</b>;
- i.e <b>X(i-1) -> Prefix(i-1)X(i)β</b> for some <b>β</b>.

Recursively, <b>Prefix(k+1) ... Prefix(n)</b> eventually reduces to the missing part of <b>α(k)</b>.

<b>Idea</b>: To recognize viable prefixes, we must:
- Recognize a sequence of partial rhs's of productions, where
- Each partial rhs can eventually reduce to part of the missing suffix of its predecessor.

# 08-03: Recognizing Viable Prefixes
1. Add a dummy production <b>S' -> S</b> to <b>G</b>;
2. The NFA states are the items of <b>G</b>
    - Including the extra production
3. For item <b>E -> α.Xβ</b> add transition <b>E -> α.Xβ</b> ->(<b>X</b>) <b>E -> αX.β</b>
4. For item <b>E -> α.Xβ</b> and production <b>X -> γ</b> add
    - <b>E -> α.Xβ</b> ->(<b>ε</b>) <b>X -> .γ</b>
5. Every state is an accepting state
6. Start state is <b>S' -> .S</b>

# 08-04: Valid Items
Item <b>X -> β.γ</b> is valid for a viable prefix <b>αβ</b> if <b>S' ->* αXω -> αβγω</b> by a right-most derivation.

After parsing <b>αβ</b>, the valid items are the possible tops of the stack of items.

An item <b>I</b> is valid for a viable prefix <b>α</b> if the DFA recognizing viable prefixes terminates on input <b>α</b> in a state <b>s</b> containing <b>I</b>

The items in <b>s</b> describe what the top of item stack might be after reading input <b>α</b>.

# 08-05: SLR Parsing
LR(0) Parsing: Assume
- stack contains <b>α</b>
- next input is <b>t</b>
- DFA on input <b>α</b> terminates in state <b>s</b>

Reduce by <b>X -> β</b> if 
- <b>s</b> contains item <b>X -> β.</b> 

Shift if
- <b>s</b> contains item <b>X -> β.tω</b>
- equivalent to saying <b>s</b> has a transition labeled <b>t</b>

LR(0) has a reduce/reduce conflict if
- Any state has two reduce items:
- <b>X -> β.</b> and <b>X -> ω.</b>

LR(0) has a shift/reduce conflict if:
- Any state has a reduce item and a shift item:
- <b>X -> β.</b> and <b>Y -> ω.tδ</b>

SLR Idea:

Reduce by <b>X -> β</b> if 
- <b>s</b> contains item <b>X -> β.</b>
- <b>t ∈ Follow(X)</b>

SLR Parsing Algorithm:
1. Let M be DFA for viable prefix of G
2. Let |x1..xn$ be initial configuration
3. Repeat until configuration is S|$
    - Let α|ω be current configuration
    - Run M on current stack α
    - if M rejects α, report parsing error
        - Stack α is not a viable prefix
    - if M accepts α with item I, let a be next input
        - Shift if <b>X -> β.aγ ∈ I</b>
        - Reduce if <b>X -> β.</b> and <b>a ∈ Follow(X)</b>
        - Report parsing error if neither applies

if there is a conflict in the last step, grammar is not SLR(k)