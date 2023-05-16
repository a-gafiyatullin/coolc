# 05-01: Introduction to Parsing
<b>Input</b>: sequence of tokens from lexer;<br>
<b>Output</b>: parse tree of the program.
# 05-02: Context Free Grammars Part 1 
- Programming language have recursive structure;
- Contex-free grammar are a natural notation for this recursive structure.

## A CFG consists of:
- A set of terminals (<b>T</b>);
- A set of non-terminals (<b>N</b>);
- A start symbol <b>S, S ∈ N</b>;
- A set of productions: <b>X -> Yi...Yn, X ∈ N, Yi ∈ N ∪ T ∪ {ε}</b>.

## Example:
S -> (S)<br>
S -> ε<br>
N = {S}<br>
T = {(,)}

Let <b>G</b> be a contex-free grammar with start symbol <b>S</b>. Then the language <b>L(G)</b> of <b>G</b> is:<br>
<b>{ a1...an | ∀i ai ∈ T & S ->* a1...an }</b>

Terminals ought to be tokens of the language.
# 05-02: Context Free Grammars Part 2
- Many grammars generate the same language;
- Tools are sensitive to the grammar.
# 05-03: Derivations Part 1
A <b>derivation</b> is a sequence of productions.<br>
A <b>derivation</b> can be drawn as a <b>tree</b>:
- Start symbol is the tree's root;
- For a production <b>X -> Y1...Yn</b> add children <b>Y1...Yn</b> to node <b>X</b>.

A parse tree has:
- Terminals at the leaves;
- Non-terminals at the interior nodes.<br>

An in-order traversal of the leaves is the original input;<br>
The parse tree shows the association of operations, the input string does no.<br>

The <b>left-most</b> derivation:
- At each step, replace the left-most non-terminal.

The <b>right-most</b> derivation do an opposite algorithm.
# 05-03: Derivations Part 2
A derivation defines a parse tree
- But one parse tree may have many derivations.
# 05-04: Ambiguity Part 1
A grammar is <b>ambiguous</b> if it has more than one parse tree for some string.
# 05-04: Ambiguity Part 3
- Impossible to convert automatically an ambiguous grammar to an unambiguous one;
- Used with care, ambiguity can simplify the grammar;
- Most tools allow precedence and associativity declarations to disambiguate grammars.
# 06-01: Error Handling
Error handler should:
- Report errors accurately and clearly;
- Recover from an error quickly;
- Not slow down compilation of valid code.

Panic mode:
- When an error is detected:
    - Discard tokens until one with a clear role is found;
    - Continue from there.
- Looking for synchronizing tokens:
    Typically the statement or expression terminators.

Error production:
- Speify known common mistakes in the grammar;
- Complicates the grammar.

Error correction:
- Try token insertions and deletions;
- Exhaustive search.
- Disadvanatages:
    - Hard to impelement;
    - Slows down parsing of correct programs;
    - "Nearby" program is not necessarily "the intended" program.

#  06-02: Abstract Syntax Trees
Abstract syntax trees:
- Like parse trees but ignore some details;
- Abbreviated as AST.

# 06-03: Recursive Descent Parsing
- The parse tree is constructed:
    - From the top;
    - From the left to right.
- Terminals are seen in order of appearance in the token streem.
- Match and backtrack on failure.

# 06-04: Recursive Descent Algorithm
Let the global <b>next</b> point to the next input token.<br>
- Define boolean function that check for a match of:
    - A given token terminal:<br>
    <code>bool term(TOKEN tok) { return *next++ == tok; }</code>
    - The nth production of S:<br>
    <code>bool Sn() {...}</code>
    - Try all productions of S:<br>
    <code>bool S() {...}</code>
- For production <b>E -> T</b>:<br>
<code>bool E1() { return T(); }</code>
- For production <b>E -> T + E</b>:<br>
<code>bool E2() { return T() && term(PLUS) && E(); }</code>
- For all productions of E (with backtracking):<br>
<code>bool E() { TOKEN *save = next; return (next = save, E1()) || (next = save, E2()); }</code>
- Functions for non-terminal T:<br>
    - <code>bool T1() { return term(INT); }</code>
    - <code>bool T2() { return term(INT) && term(TIMES) && T(); }</code>
    - <code>bool T3() { return term(OPEN) && E() && term(CLOSE); }</code>
    - <code>bool T() { TOKEN *save = next; return (next = save, T1()) || (next = save, T2()) || (next = save, T3()); }</code>
- To start the parser:
    - Initialize <b>next</b> to point to first token;
    - Invoke <b>E()</b>.

# 06-04-1: Recursive Descent Limitations
- Presented recursive descent algorithm is not general;
- Sufficient for grammars where for any non-terminal at most one production can succeed;
- The example grammar can be rewritten to work with the presented algorithm:
    - By left factoring.

# 06-05: Left Recursion Part 1
- A <b>left-recursive</b> grammar has a non-terminal S: <b>S ->+ Sα</b> for some <b>α</b>.
- Recursive descent does not work with left-recursive grammars.
- Rewrite using right-recursion:<br>
<b>Let S -> Sα | β</b>,<br> then<br>
<b>S -> βS'<br>S->αS' | ε</b><br> eliminates left-recursion.