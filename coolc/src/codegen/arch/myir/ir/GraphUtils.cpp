#include "GraphUtils.hpp"
#include "GraphUtils.inline.hpp"
#include "utils/Utils.h"
#include <algorithm>
#include <string>

using namespace myir;

std::vector<block> GraphUtils::postorder(const block &root)
{
    std::vector<myir::block> traversal;
    GraphUtils::dfs<GraphUtils::POSTORDER>(
        root, [&traversal](const myir::block &b, const myir::block &s) { traversal.push_back(b); });

    clear_visited(traversal);
    return traversal;
}

std::vector<block> GraphUtils::preorder(const block &root)
{
    std::vector<block> traversal;
    GraphUtils::dfs<GraphUtils::PREORDER>(
        root, [&traversal](const myir::block &b, const myir::block &s) { traversal.push_back(b); });

    clear_visited(traversal);
    return traversal;
}

void GraphUtils::clear_visited(const std::vector<myir::block> &blocks)
{
    for (auto b : blocks)
    {
        b->_is_visited = false;
    }
}

block GraphUtils::dominance_intersect(const block &b1, const block &b2, const std::unordered_map<block, block> &doms)
{
    auto finger1 = b1;
    auto finger2 = b2;

    while (finger1->postorder() != finger2->postorder())
    {
        while (finger1->postorder() < finger2->postorder())
        {
            finger1 = doms.at(finger1);
        }

        while (finger2->postorder() < finger1->postorder())
        {
            finger2 = doms.at(finger2);
        }
    }

    return finger1;
}

std::unordered_map<block, block> GraphUtils::dominance(const block &root)
{
    auto rpo = postorder(root);

    int num = 0;
    for (auto b : rpo)
    {
        b->_postorder_num = num++;
    }

    rpo.erase(find(rpo.begin(), rpo.end(), root));
    std::reverse(rpo.begin(), rpo.end());

    std::unordered_map<block, block> doms; // dominance array
    doms.reserve(rpo.size());
    for (auto b : rpo) // initialize it
    {
        doms[b] = nullptr;
    }
    doms[root] = root;

    bool changed = true;

    while (changed)
    {
        changed = false;
        for (auto b : rpo)
        {
            // first (processed) predecessor of b
            auto new_idom_iter =
                std::find_if(b->_preds.begin(), b->_preds.end(), [&doms](const block &b) { return doms.at(b); });
            assert(new_idom_iter != b->_preds.end());
            auto new_idom = *new_idom_iter;

            for (auto p : b->_preds)
            {
                if (doms[p])
                {
                    new_idom = dominance_intersect(p, new_idom, doms);
                }
            }

            if (doms[b] != new_idom)
            {
                doms[b] = new_idom;
                changed = true;
            }
        }
    }

    DEBUG_ONLY(dump_dominance(doms));
    return doms;
}

std::unordered_map<block, std::set<block>> GraphUtils::dominance_frontier(const block &root)
{
    std::unordered_map<block, std::set<block>> df;

    auto doms = dominance(root);
    df.reserve(doms.size());

    for (auto &[b, d] : doms)
    {
        if (b->_preds.size() >= 2)
        {
            for (auto &p : b->_preds)
            {
                auto runner = p;
                while (runner != d)
                {
                    df[runner].insert(b);
                    runner = doms[runner];
                }
            }
        }
    }

    DEBUG_ONLY(dump_dominance_frontier(df));
    return df;
}

#ifdef DEBUG
void GraphUtils::dump_dominance(const std::unordered_map<block, block> &doms)
{
    if (!PrintDominanceInfo)
    {
        return;
    }

    std::cout << "Dominance:\n";

    for (auto &[b, d] : doms)
    {
        std::cout << "  Dom(" << b->name() << ") = " << d->name() << "\n";
    }
}

void GraphUtils::dump_dominance_frontier(const std::unordered_map<block, std::set<block>> &df)
{
    if (!PrintDominanceInfo)
    {
        return;
    }

    std::string s = "Dominance frontier:\n";
    for (auto &[b, dfset] : df)
    {
        s += "  DF(" + b->name() + ") = [";
        for (auto dfb : dfset)
        {
            s += dfb->name() + ", ";
        }

        if (s.back() == ' ')
        {
            s = s.substr(0, s.length() - 2); // trim ", "
        }

        s += "]\n";
    }

    std::cout << s;
}
#endif // DEBUG