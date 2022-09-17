# 16-01: Register Allocation
Temporaries **t1** and **t2** can share the same register if at any point in the program at most one of **t1** or **t2** is live.

Construct an undirected graph:
- A node for each temporary;
- An edge between **t1** and **t2** if they are live simultaneously at some point in the program.

This is the **register interference graph** (RIG):
- Two temporaries can be allocated to the same register if there is no edge connecting them.

# 16-02: Graph Coloring
A **coloring of a graph** is an assignment of colors to nodes, such that nodes connected by an edge have different colors;

A graph is **k-colorable** if it has a coloring with k colors.

The following works well in practice:
- Pick a node **t** with fewer than **k** neigbours;
- Put **t** on a stack and remove it from the RIG;
- Repeat until the graph is empty.

Assign colors to nodes on the stack:
- Start with the last node added;
- At each step pick a color different from those assigned to already colored neigbours.

# 16-03: Spilling
- Pick a node (**f**) as a candidate for spilling;
- Remove **f** and continue the simplification;
- Eventually we must assign a color to **f**;
- We hope that among the **k** neigbours of **f** we use less than **k - 1** color -> **optimistic coloring**;
- if optimistic coloring fails, we spill **f**:
  - Allocate a memeory location for **f**;

Spilling reduces the live range of **f**:
- And thus reduces its interferences;
 - Which results in fewer RIG neigbours.

Additional spills might be required before a coloring is found.

Heuristics what to spill:
- Spill temporaries with most conflicts;
- Spill temporaries with few definitions and uses;
- Avoid spilling in inner loops.

# 16-04: Managing Caches
**Loop interchange** optimization.

# 17-01: Automatic Memory Management
**Observation**: a program can use only the objects that it can find.

Every GC scheme has the following steps:
1. Allocate space as needed for new objects;
2. When space runs out:
    - Compute what objects might be used again(generally by tracing objects reachable from a set of "root" registers);
    - Free the space used by objects not found in the previous step.

# 17-02: Mark and Sweep
When memory runs out, GC executes two phases:
- the **mark** phase: traces reachable objects;
- the **sweep** phase: collect garbage objects.

Every object has an extra bit: the mark bit:
- reserved for memory management;
- initially the mark bit is 0;
- set to 1 for the reachable objects in the mark phase.

Space for a new object is allocated from the free list:
- a block large enough is picked;
- an area of the necessary size is allocated from it;
- the left-over is put back in the free list.

Mark and sweep can fragment the memory.

# 17-03: Stop and Copy
Memory is organized into two areas:
- **old space**: used for allocation;
- **new space**: used as a reserve for GC.

The heap pointer points to the next free word in the old space;

Allocation just advances the heap pointer.

GC starts when the old space is full.

Copies all reachable objects from old space into new space:
- garbage is left behind;
- after the copy phase the ne wspace ises less space than the old one before the collection.

After the copy the roles of the old and new spaces are reversed and the program resumes.

As we copy an object we store in the old copy a **forwarding pointer** to the new copy:
- when we later reach an object with a forwarding pointer we know it was already copied.

We still have the issue of how to implement the traversal without using extra space:
- partion the new space in three contiguous regions:
  - **copied and scanned**: copied objects whose pointer fields have been followed;
  - **copied**: copied objects whose pointer fields have not been followed yet;
  - **empty**.

**Step 1**: Copy the objects pointed to by roots and set forwarding pointers;

**Step 2**: Follow the pointer in the next unscanned object (A):
- copy the pointed-to objects;
- fix the pointer in A;
- set forwarding pointer.

Each object is processed entirely before any objects it points to.

**Stop and Copy** is generally believed to be the fastest GC technique.

# 17-04: Conservative Collection
If a memeory word looks like a pointer it is considered a pointer:
- it must be aligned;
- It must point to a valid address in the data segment

All such pointers are followed and we overestimate the set of reachable objects.

# 17-05: Reference Counting
Store in each object the number of pointers to that object:
- this is the reference count.

Each assignment operation manipulates the reference count.

Advantages:
- easy to implement;
- collect garbage incrementally without large pauses in the execution;
  
Disadvantages:
- cannot collect circular structures;
- manipulating reference count at each assignment is very slow.