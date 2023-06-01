#pragma once

#include "IR.hpp"
#include <functional>
#include <set>
#include <string>

namespace myir
{

class CFG
{
  public:
    enum DFSType
    {
        PREORDER,         // visit node before its successors
        INORDER,          // visit every pair (node, successor)
        POSTORDER,        // visit node after its successors
        REVERSE_POSTORDER // reversed POSTORDER
    };

  private:
    block _root;

    // traversals
    std::vector<block> _postorder;
    std::vector<block> _preorder;
    std::vector<block> _reverse_postorder;

    // dominance info
    std::unordered_map<block, block> _dominance;
    std::unordered_map<block, std::set<block>> _dominance_frontier;

    block dominance_intersect(const block &b1, const block &b2);

    // common DFS
    template <DFSType type>
    void dfs(const block &root, const std::function<void(const block &s, const block &d)> &visitor);

#ifdef DEBUG
    void dump_dominance();
    void dump_dominance_frontier();
#endif // DEBUG

  public:
    // graph traversals
    template <DFSType type> std::vector<block> &traversal();

    // helpers
    void clear_visited();

    // setters
    inline void set_cfg(const block &b) { _root = b; }

    // getters
    inline block root() const { return _root; }
    inline bool empty() const { return !root(); }

    // for every block gather all blocks that dominate it
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    std::unordered_map<block, block> &dominance();

    // for every block gather all blocks in its DF
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    std::unordered_map<block, std::set<block>> &dominance_frontier();

    // debugging
    std::string dump() const;
};

// nice printing
extern void trim(std::string &where, const std::string &what);
}; // namespace myir