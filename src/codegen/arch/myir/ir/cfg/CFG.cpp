#include "CFG.hpp"
#include "utils/Utils.h"
#include <iostream>
#include <stack>

using namespace myir;

void myir::trim(std::string &where, const std::string &what)
{
    const int pos = where.length() - what.length();
    if (pos >= 0 && where.substr(pos) == what)
    {
        where = where.substr(0, pos);
    }
}

Block *CFG::dominance_intersect(const DominanceInfo &info, Block *b1, Block *b2)
{
    auto *finger1 = b1;
    auto *finger2 = b2;

    while (finger1->postorder() != finger2->postorder())
    {
        while (finger1->postorder() < finger2->postorder())
        {
            finger1 = info.dominance().at(finger1);
        }

        while (finger2->postorder() < finger1->postorder())
        {
            finger2 = info.dominance().at(finger2);
        }
    }

    return finger1;
}

CFG::DominanceInfo CFG::dominance()
{
    DominanceInfo info;
    dominance(info);

    DEBUG_ONLY(dump_dominance(info));
    return info;
}

void CFG::dominance(DominanceInfo &info)
{
    // copy here
    auto rpo = traversal<REVERSE_POSTORDER>();
    rpo.erase(find(rpo.begin(), rpo.end(), _root));

    for (auto *b : rpo) // initialize it
    {
        info._dominance[b] = nullptr;
    }
    info._dominance[_root] = _root;

    bool changed = true;

    while (changed)
    {
        changed = false;
        for (auto *b : rpo)
        {
            // first (processed) predecessor of b
            auto new_idom_iter =
                std::find_if(b->_preds.begin(), b->_preds.end(), [&info](Block *b) { return info._dominance.at(b); });
            assert(new_idom_iter != b->_preds.end());
            auto *new_idom = *new_idom_iter;

            for (auto *p : b->_preds)
            {
                if (info._dominance[p])
                {
                    new_idom = dominance_intersect(info, p, new_idom);
                }
            }

            if (info._dominance[b] != new_idom)
            {
                info._dominance[b] = new_idom;
                changed = true;
            }
        }
    }

    dominator_tree(info);
    dominance_frontier(info);
}

void CFG::dominator_tree(DominanceInfo &info)
{
    assert(!info._dominance.empty());

    // construct the dominator tree from dominance info
    for (auto &[bb, dom] : info.dominance())
    {
        info._dominator_tree[dom].push_back(bb);
    }

    if (info._dominator_tree.contains(_root))
    {
        auto &rootset = info._dominator_tree.at(_root);
        rootset.erase(find(rootset.begin(), rootset.end(), _root));
    }
}

bool CFG::DominanceInfo::dominate(Block *dominator, Block *dominatee) const
{
    if (dominator == dominatee)
    {
        return true;
    }

    auto &dt = dominator_tree();

    std::stack<Block *> s;
    s.push(dominator);

    while (!s.empty())
    {
        auto *bb = s.top();
        s.pop();

        if (!dt.contains(bb))
        {
            continue;
        }

        for (auto *child : dt.at(bb))
        {
            if (child == dominatee)
            {
                return true;
            }
            s.push(child);
        }
    }

    return false;
}

void CFG::dominance_frontier(DominanceInfo &info)
{
    assert(!info._dominance.empty());

    for (auto &[b, d] : info.dominance())
    {
        if (b->_preds.size() >= 2)
        {
            for (auto &p : b->_preds)
            {
                auto *runner = p;
                while (runner != d)
                {
                    info._dominance_frontier[runner].insert(b);
                    runner = info._dominance[runner];
                }
            }
        }
    }
}

#ifdef DEBUG
void CFG::dump_dominance(const DominanceInfo &info)
{
    if (!PrintDominanceInfo)
    {
        return;
    }

    std::cout << "Dominance:\n";

    for (auto &[b, d] : info.dominance())
    {
        std::cout << "  Dom(" << b->name() << ") = " << d->name() << "\n";
    }

    std::cout << "Dominator Tree:\n";
    for (auto &[b, d] : info.dominator_tree())
    {
        std::string str = " " + b->name() + " dominates [";
        for (auto *dominatee : d)
        {
            str += dominatee->name() + ", ";
        }

        trim(str, ", ");
        std::cout << str + "]\n";
    }

    std::string s = "Dominance frontier:\n";
    for (auto &[b, dfset] : info.dominance_frontier())
    {
        s += "  DF(" + b->name() + ") = [";
        for (auto *dfb : dfset)
        {
            s += dfb->name() + ", ";
        }

        trim(s, ", ");
        s += "]\n";
    }

    std::cout << s;
}
#endif // DEBUG