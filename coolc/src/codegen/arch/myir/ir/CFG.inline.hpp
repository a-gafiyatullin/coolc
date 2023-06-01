#include "CFG.hpp"
using namespace myir;

template <CFG::DFSType type>
void CFG::dfs(const block &b, const std::function<void(const block &s, const block &d)> &visitor)
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

template <CFG::DFSType type> std::vector<block> &CFG::traversal()
{
    bool need_clear = false;

    if constexpr (type == POSTORDER || type == REVERSE_POSTORDER)
    {
        if (_postorder.empty())
        {
            need_clear = true;

            dfs<POSTORDER>(_root, [this](const myir::block &b, const myir::block &s) { _postorder.push_back(b); });
            _reverse_postorder.resize(_postorder.size());
            std::reverse_copy(_postorder.begin(), _postorder.end(), _reverse_postorder.begin());

            // set postorder numbers
            int num = 0;
            std::for_each(_postorder.begin(), _postorder.end(), [&num](const block &b) { b->_postorder_num = num++; });
        }
    }
    else
    {
        if (_preorder.empty())
        {
            need_clear = true;
            dfs<PREORDER>(_root, [this](const myir::block &b, const myir::block &s) { _preorder.push_back(b); });
        }
    }

    if (need_clear)
    {
        clear_visited();
    }

    if constexpr (type == POSTORDER)
    {
        return _postorder;
    }
    else if constexpr (type == PREORDER)
    {
        return _preorder;
    }
    else
    {
        return _reverse_postorder;
    }
}