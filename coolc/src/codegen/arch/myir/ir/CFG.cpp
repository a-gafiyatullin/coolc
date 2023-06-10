#include "CFG.hpp"
#include "utils/Utils.h"

using namespace myir;

void myir::trim(std::string &where, const std::string &what)
{
    int pos = where.length() - what.length();
    if (pos >= 0 && where.substr(pos) == what)
    {
        where = where.substr(0, pos);
    }
}

void CFG::clear_visited()
{
    if (!_preorder.empty())
    {
        for (auto *b : _preorder)
        {
            b->_is_visited = false;
        }
    }

    for (auto *b : _postorder)
    {
        b->_is_visited = false;
    }
}

Block *CFG::dominance_intersect(Block *b1, Block *b2)
{
    auto *finger1 = b1;
    auto *finger2 = b2;

    while (finger1->postorder() != finger2->postorder())
    {
        while (finger1->postorder() < finger2->postorder())
        {
            finger1 = _dominance.at(finger1);
        }

        while (finger2->postorder() < finger1->postorder())
        {
            finger2 = _dominance.at(finger2);
        }
    }

    return finger1;
}

allocator::irunordered_map<Block *, Block *> &CFG::dominance()
{
    if (!_dominance.empty())
    {
        return _dominance;
    }

    // copy here
    auto rpo = traversal<REVERSE_POSTORDER>();
    rpo.erase(find(rpo.begin(), rpo.end(), _root));

    for (auto *b : rpo) // initialize it
    {
        _dominance[b] = nullptr;
    }
    _dominance[_root] = _root;

    bool changed = true;

    while (changed)
    {
        changed = false;
        for (auto *b : rpo)
        {
            // first (processed) predecessor of b
            auto new_idom_iter =
                std::find_if(b->_preds.begin(), b->_preds.end(), [this](Block *b) { return _dominance.at(b); });
            assert(new_idom_iter != b->_preds.end());
            auto *new_idom = *new_idom_iter;

            for (auto *p : b->_preds)
            {
                if (_dominance[p])
                {
                    new_idom = dominance_intersect(p, new_idom);
                }
            }

            if (_dominance[b] != new_idom)
            {
                _dominance[b] = new_idom;
                changed = true;
            }
        }
    }

    // also construct the dominator tree from dominance info
    for (auto &[bb, dom] : _dominance)
    {
        if (!_dominator_tree.contains(dom))
        {
            _dominator_tree.insert({dom, allocator::irvector<Block *>(ALLOC1)});
        }

        _dominator_tree.at(dom).push_back(bb);
    }

    if (_dominator_tree.contains(_root))
    {
        auto &rootset = _dominator_tree.at(_root);
        rootset.erase(find(rootset.begin(), rootset.end(), _root));
    }

    DEBUG_ONLY(dump_dominance());
    return _dominance;
}

allocator::irunordered_map<Block *, allocator::irvector<Block *>> &CFG::dominator_tree()
{
    dominance();
    return _dominator_tree;
}

bool CFG::dominate(Block *dominator, Block *dominatee)
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

allocator::irunordered_map<Block *, allocator::irset<Block *>> &CFG::dominance_frontier()
{

    if (!_dominance_frontier.empty())
    {
        return _dominance_frontier;
    }

    dominance();

    for (auto &[b, d] : _dominance)
    {
        if (b->_preds.size() >= 2)
        {
            for (auto &p : b->_preds)
            {
                auto *runner = p;
                while (runner != d)
                {
                    if (!_dominance_frontier.contains(runner))
                    {
                        _dominance_frontier.insert({runner, allocator::irset<Block *>(ALLOC1)});
                    }

                    _dominance_frontier.at(runner).insert(b);
                    runner = _dominance[runner];
                }
            }
        }
    }

    DEBUG_ONLY(dump_dominance_frontier());
    return _dominance_frontier;
}

#ifdef DEBUG
void CFG::dump_dominance()
{
    if (!PrintDominanceInfo)
    {
        return;
    }

    std::cout << "Dominance:\n";

    for (auto &[b, d] : _dominance)
    {
        std::cout << "  Dom(" << b->name() << ") = " << d->name() << "\n";
    }

    std::cout << "Dominator Tree:\n";
    for (auto &[b, d] : _dominator_tree)
    {
        std::string str = " " + b->name() + " dominates [";
        for (auto *dominatee : d)
        {
            str += dominatee->name() + ", ";
        }

        trim(str, ", ");
        std::cout << str + "]\n";
    }
}

void CFG::dump_dominance_frontier()
{
    if (!TraceSSAConstruction)
    {
        return;
    }

    std::string s = "Dominance frontier:\n";
    for (auto &[b, dfset] : _dominance_frontier)
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