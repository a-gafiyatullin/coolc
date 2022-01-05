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