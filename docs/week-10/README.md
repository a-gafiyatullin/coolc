# 18-02: Java Arrays
Having multiple aliases to updateable locations with different types is unsound!

- Disallow subtyping through arrays: **B[] < A[] if B = A**
- Java fixes the problem by checking each array assignment at runtime for type correctness.

# 18-03: Java Exceptions
When we encounter a **try**:
- Mark current location in the stack.

When we **throw** an **exception**:
- Unwind the stack to the first **try**;
- Execute corresponding **catch**.

# 18-04: Java Interfaces
Methods in classes implementing interfaces need not be at fixed offsets.

Dispathes **e.f(...)** where **e** has an interface type are more complex than usual:
- Because methods don't live at fixed offsets.

One approach:
- Each class implementing an interface has a lookup table **method names -> methods**;
- Hash method names for faster lookup:
  - hashes computed at compile time.

# 18-05: Java Coercions
Java distinguishes two kinds of coercions & casts:
- **Widening** always succeed (**int -> float**);
- **Narrowing** may fail if data can't be converted to desired type (**float -> int**, downcast).

Narrowing casts must be explicit;

Widening casts/coercions can be implicit.

# 18-06: Java Threads
In **synchronized methods**, **this** is locked.

Writes of values are atomic, except doubles (not **volatile**).