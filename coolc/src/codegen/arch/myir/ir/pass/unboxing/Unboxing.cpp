#include "Unboxing.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"

using namespace myir;

void Unboxing::run(Function *func)
{
    std::vector<bool> processed(Instruction::max_id() * 2); // can create new instructions
    replace_args(processed);
    replace_lets(processed);
}

void Unboxing::replace_args(std::vector<bool> &processed)
{
    std::stack<Instruction *> replace;

    auto *entry = _cfg->root();

    for (auto *param : _func->params())
    {
        if (param->type() == INTEGER || param->type() == BOOLEAN)
        {
            // prepare an operand instead of the object for primitive
            auto *value = new Operand(INT64);
            auto *offset = new Constant(codegen::HeaderLayoutOffsets::FieldOffset, UINT64);
            auto *load = new Load(value, param, offset, entry);

            _cfg->root()->append_front(load);

            std::vector<Instruction *> for_uses_update;

            // update uses of this INTEGER or BOOLEAN with a new operand
            for (auto *use : param->uses())
            {
                if (use == load || Instruction::isa<Call>(use))
                {
                    assert(use == load || !Instruction::as<Call>(use)->callee()->is_init());
                    continue;
                }

                for_uses_update.push_back(use);
            }

            for (auto *inst : for_uses_update)
            {
                inst->update_use(param, value);
                replace.push(inst);
            }
        }
    }

    replace_uses(replace, processed);
}

void Unboxing::replace_lets(std::vector<bool> &processed)
{
    std::stack<Instruction *> replace;

    for (auto *b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        std::vector<Instruction *> for_append;

        // search for moves of Integer/Boolean constants
        for (auto *inst : b->insts())
        {
            if (Instruction::isa<Move>(inst))
            {
                auto *oper = inst->use(0);
                if ((inst->def()->type() != INTEGER && inst->def()->type() != BOOLEAN) ||
                    !Operand::isa<GlobalConstant>(oper))
                {
                    continue;
                }

                auto *initializer = Operand::as<GlobalConstant>(oper);
                // found instruction : value <- global_const
                auto *value = initializer->field(codegen::HeaderLayoutOffsets::FieldOffset);

                // create a new move and use its def in all uses of the original move except calls
                auto *move = new Move(new Operand(value->type()), value, b);

                std::vector<Instruction *> for_uses_update;

                for (auto *use : inst->def()->uses())
                {
                    if (!Instruction::isa<Call>(use))
                    {
                        for_uses_update.push_back(use);
                    }
                }

                for (auto *use : for_uses_update)
                {
                    use->update_use(inst->def(), move->def());

                    // users of the def already has corerct use
                    replace.push(use);
                }

                for_append.push_back(move);
            }
        }

        for (auto *newinst : for_append)
        {
            b->append_front(newinst);
        }
    }

    replace_uses(replace, processed);
}

void Unboxing::replace_uses(std::stack<Instruction *> &s, std::vector<bool> &processed)
{
    std::vector<Instruction *> for_delete;
    while (!s.empty())
    {
        auto *inst = s.top();
        s.pop();

        if (processed[inst->id()])
        {
            continue;
        }

        processed[inst->id()] = true;

        if (Instruction::isa<Load>(inst))
        {
            replace_load(Instruction::as<Load>(inst), s);
        }
        else if (Instruction::isa<Store>(inst))
        {
            replace_store(Instruction::as<Store>(inst), s);
        }
        else if (Instruction::isa<Call>(inst))
        {
            SHOULD_NOT_REACH_HERE();
        }
        else
        {
            if (!inst->def())
            {
                continue;
            }

            for (auto *use : inst->def()->uses())
            {
                s.push(use);
            }
        }
    }
}

void Unboxing::replace_load(Load *load, std::stack<Instruction *> &s)
{
    // load from Integer object. Have to be replaced by moves
    auto *result = load->def();
    auto *object = load->use(0); // former object and now it's an INT64
    auto *offset = load->use(1);
    auto offset_val = Operand::as<Constant>(offset)->value();

    auto *block = load->holder();

    // all acceses to Integer objects by constant offsets
    switch (offset_val)
    {
    case codegen::HeaderLayoutOffsets::FieldOffset: {
        // don't change def. Will be eliminated by copy propagation
        result->erase_def(load);
        auto *move = new Move(result, object, block);
        block->append_instead(load, move);
        break;
    }
    default:
        SHOULD_NOT_REACH_HERE();
    }

    for (auto *use : result->uses())
    {
        s.push(use);
    }
}

void Unboxing::replace_store(Store *store, std::stack<Instruction *> &s)
{
    // store to Integer object. Replace use of newly allocated object with a new def
    auto *object = store->use(0);
    auto *offset = store->use(1);
    auto *value = store->use(2);

    // access can be only to value field
    auto offset_val = Operand::as<Constant>(offset)->value();
    assert(offset_val == codegen::HeaderLayoutOffsets::FieldOffset);

    auto *block = store->holder();

    std::vector<Instruction *> for_delete;
    for_delete.push_back(store);

    std::vector<Instruction *> for_uses_update;

    for (auto *use : object->uses())
    {
        if (use == store)
        {
            // we have done with the original store
            continue;
        }

        // this newly allocated object can be used by another load, as value of the store, as call argument
        if (Instruction::isa<Call>(use))
        {
            auto *callee = Instruction::as<Call>(use)->callee();

            // init can be deleted
            if (callee->is_init())
            {
                for_delete.push_back(use);
            }
            else
            {
                SHOULD_NOT_REACH_HERE();
            }
        }
        else if (Instruction::isa<Store>(use))
        {
            // this object can be used as a value for a store or base of a store

            // In SSA form store of the object can be used only for fields:
            // it means that we have to leave this object and it's allocation
            if (use->use(2) != object)
            {
                SHOULD_NOT_REACH_HERE();
            }
            else
            {
                for_delete.clear();
            }
        }
        else
        {
            // next load/move from/of newly allocated object. Replace it with INT64 value
            for_uses_update.push_back(use);
        }
    }

    for (auto *inst : for_uses_update)
    {
        inst->update_use(object, value);
        s.push(inst);
    }

    // We can delete object allocaion and initialization
    if (!for_delete.empty())
    {
        for_delete.push_back(object->def());
        Block::erase(for_delete);
    }
}