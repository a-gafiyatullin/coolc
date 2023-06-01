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
        for (auto b : _preorder)
        {
            b->_is_visited = false;
        }
    }

    for (auto b : _postorder)
    {
        b->_is_visited = false;
    }
}

block CFG::dominance_intersect(const block &b1, const block &b2)
{
    auto finger1 = b1;
    auto finger2 = b2;

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

std::unordered_map<block, block> &CFG::dominance()
{
    if (!_dominance.empty())
    {
        return _dominance;
    }

    // copy here
    auto rpo = traversal<REVERSE_POSTORDER>();
    rpo.erase(find(rpo.begin(), rpo.end(), _root));

    _dominance.reserve(rpo.size());
    for (auto b : rpo) // initialize it
    {
        _dominance[b] = nullptr;
    }
    _dominance[_root] = _root;

    bool changed = true;

    while (changed)
    {
        changed = false;
        for (auto b : rpo)
        {
            // first (processed) predecessor of b
            auto new_idom_iter =
                std::find_if(b->_preds.begin(), b->_preds.end(), [this](const block &b) { return _dominance.at(b); });
            assert(new_idom_iter != b->_preds.end());
            auto new_idom = *new_idom_iter;

            for (auto p : b->_preds)
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

    DEBUG_ONLY(dump_dominance());
    return _dominance;
}

std::unordered_map<block, std::set<block>> &CFG::dominance_frontier()
{

    if (!_dominance_frontier.empty())
    {
        return _dominance_frontier;
    }

    dominance();
    _dominance_frontier.reserve(_dominance.size());

    for (auto &[b, d] : _dominance)
    {
        if (b->_preds.size() >= 2)
        {
            for (auto &p : b->_preds)
            {
                auto runner = p;
                while (runner != d)
                {
                    _dominance_frontier[runner].insert(b);
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
        for (auto dfb : dfset)
        {
            s += dfb->name() + ", ";
        }

        trim(s, ", ");
        s += "]\n";
    }

    std::cout << s;
}
#endif // DEBUG