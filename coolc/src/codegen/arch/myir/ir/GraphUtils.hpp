#pragma once

#include "IR.hpp"
#include <functional>
#include <set>

namespace myir
{

class GraphUtils
{
  private:
    static block dominance_intersect(const block &b1, const block &b2, const std::unordered_map<block, block> &doms);

#ifdef DEBUG
    static void dump_dominance(const std::unordered_map<block, block> &doms);
    static void dump_dominance_frontier(const std::unordered_map<block, std::set<block>> &df);
#endif // DEBUG

  public:
    enum DFSType
    {
        PREORDER, // visit node before its successors
        INORDER,  // visit every pair (node, successor)
        POSTORDER // visit node after its successors
    };

    // common DFS
    template <DFSType type>
    static void dfs(const block &root, const std::function<void(const block &s, const block &d)> &visitor);

    // graph traversals
    static std::vector<block> postorder(const block &root);
    static std::vector<block> preorder(const block &root);

    // helpers
    static void clear_visited(const std::vector<block> &blocks);

    // for every block gather all blocks that dominate it
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    static std::unordered_map<block, block> dominance(const block &root);

    // for every block gather all blocks in its DF
    // Implements approach was described in "A Simple, Fast Dominance Algorithm"
    // by Keith D. Cooper, Timothy J. Harvey, and Ken Kennedy
    static std::unordered_map<block, std::set<block>> dominance_frontier(const block &root);
};
}; // namespace myir