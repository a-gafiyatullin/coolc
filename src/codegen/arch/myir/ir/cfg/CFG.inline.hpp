#include "CFG.hpp"

template <myir::CFG::DFSType type>
void myir::CFG::dfs(Block *b, std::vector<bool> &bitset, const std::function<void(Block *s, Block *d)> &visitor)
{
    if (bitset[b->id()])
    {
        return;
    }

    bitset[b->id()] = true;

    if constexpr (type == PREORDER)
    {
        visitor(b, nullptr);
    }

    for (auto *s : b->_succs)
    {
        if constexpr (type == INORDER)
        {
            visitor(b, s);
        }

        dfs<type>(s, bitset, visitor);
    }

    if constexpr (type == POSTORDER)
    {
        visitor(b, nullptr);
    }
}

template <myir::CFG::DFSType type> std::vector<myir::Block *> myir::CFG::traversal()
{
    // note that modern C++ compilers optimize it to bitset
    std::vector<bool> bitset(Block::max_id());
    std::vector<Block *> traversal;

    if constexpr (type == POSTORDER || type == REVERSE_POSTORDER)
    {
        dfs<POSTORDER>(_root, bitset, [&traversal](Block *b, Block *s) { traversal.push_back(b); });

        // set postorder numbers
        int num = 0;
        std::for_each(traversal.begin(), traversal.end(), [&num](Block *b) { b->_postorder_num = num++; });

        if constexpr (type == REVERSE_POSTORDER)
        {
            std::reverse(traversal.begin(), traversal.end());
        }
    }
    else
    {
        dfs<PREORDER>(_root, bitset, [&traversal](Block *b, Block *s) { traversal.push_back(b); });
    }

    return traversal;
}
