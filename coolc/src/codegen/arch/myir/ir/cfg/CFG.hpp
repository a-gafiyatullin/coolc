#pragma once

#include "codegen/arch/myir/ir/IR.hpp"
#include <functional>

namespace myir
{

class CFG : public IRObject
{
  public:
    enum DFSType
    {
        PREORDER,         // visit node before its successors
        INORDER,          // visit every pair (node, successor)
        POSTORDER,        // visit node after its successors
        REVERSE_POSTORDER // reversed POSTORDER
    };

    // hold all info about dominators in one structure
    struct DominanceInfo
    {
        friend class CFG;

      private:
        std::unordered_map<Block *, Block *> _dominance;
        std::unordered_map<Block *, std::vector<Block *>> _dominator_tree;
        std::unordered_map<Block *, std::set<Block *>> _dominance_frontier;

      public:
        inline auto &dominance() const { return _dominance; }
        inline auto &dominator_tree() const { return _dominator_tree; }
        inline auto &dominance_frontier() const { return _dominance_frontier; }

        bool dominate(Block *dominator, Block *dominatee) const;
    };

  private:
    Block *_root;

    // common DFS
    template <DFSType type>
    void dfs(Block *root, std::vector<bool> &bitset, const std::function<void(Block *s, Block *d)> &visitor);

    // for every Block* gather all blocks that dominate it
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    void dominance(DominanceInfo &info);
    void dominator_tree(DominanceInfo &info);

    // for every Block* gather all blocks in its DF
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    void dominance_frontier(DominanceInfo &info);

    // helper for dominance info creation
    Block *dominance_intersect(const DominanceInfo &info, Block *b1, Block *b2);

#ifdef DEBUG
    void dump_dominance(const DominanceInfo &info);
#endif // DEBUG

  public:
    CFG() : _root(nullptr) {}

    // graph traversals
    template <DFSType type> std::vector<Block *> traversal();

    // setters
    inline void set_cfg(Block *b) { _root = b; }

    // getters
    inline Block *root() const { return _root; }
    inline bool empty() const { return !root(); }

    // get dominance info about this CFG
    DominanceInfo dominance();

    // debugging
    std::string dump() const;
};

// nice printing
extern void trim(std::string &where, const std::string &what);
}; // namespace myir