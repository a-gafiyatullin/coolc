#include "GraphUtils.hpp"

using namespace myir;

template <GraphUtils::DFSType type>
void GraphUtils::dfs(const block &b, const std::function<void(const block &s, const block &d)> &visitor)
{
    if (b->_is_visited)
    {
        return;
    }

    b->_is_visited = true;

    if constexpr (type == PREORDER)
    {
        visitor(b, nullptr);
    }

    for (auto s : b->_succs)
    {
        if constexpr (type == INORDER)
        {
            visitor(b, s);
        }

        dfs<type>(s, visitor);
    }

    if constexpr (type == POSTORDER)
    {
        visitor(b, nullptr);
    }
}