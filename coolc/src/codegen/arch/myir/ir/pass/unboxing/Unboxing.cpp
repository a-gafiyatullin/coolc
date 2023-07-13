#include "Unboxing.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"
#include "codegen/arch/myir/ir/cfg/CFG.hpp"
#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"

using namespace myir;

void Unboxing::run(Function *func)
{
    std::vector<bool> processed(Instruction::max_id() * 2); // can create new instructions
    prepare_args(processed);
    prepare_lets_and_calls(processed);
}

std::shared_ptr<codegen::Klass> Unboxing::operand_to_klass(OperandType type) const
{
    switch (type)
    {
    case INTEGER:
        return _builder->klass(BaseClassesNames[BaseClasses::INT]);
    case BOOLEAN:
        return _builder->klass(BaseClassesNames[BaseClasses::BOOL]);
    }

    SHOULD_NOT_REACH_HERE();
}

bool Unboxing::need_unboxing(Operand *value) const
{
    // We don't need unboxing if value was used by calls only (aka call argument)
    for (auto *use : value->uses())
    {
        if (!Instruction::isa<Call>(use))
        {
            return true;
        }
    }

    return false;
}

Operand *Unboxing::allocate_primitive(Instruction *before, Operand *value,
                                      const std::shared_ptr<ast::Type> &klass_type) const
{
    auto &klass = _builder->klass(klass_type->_string);

    auto *func_alloca = _runtime.symbol_by_id(codegen::RuntimeMyIR::RuntimeMyIRSymbols::GC_ALLOC)->_func;
    auto *func_init = _module.get<myir::Function>(klass->init_method());

    // prepare tag, size and dispatch table
    auto *tag = new myir::Constant(klass->tag(), _runtime.header_elem_type(codegen::HeaderLayout::Tag));
    auto *size = new myir::Constant(klass->size(), _runtime.header_elem_type(codegen::HeaderLayout::Size));
    auto *disp_tab = _data.class_disp_tab(klass);

    auto *block = before->holder();

    // call allocation
    auto *object = new Operand(_data.ast_to_ir_type(klass_type));
    auto *alloca = new Call(func_alloca, object, {func_alloca, tag, size, disp_tab});
    block->append_before(before, alloca);

    // call init
    auto *init = new Call(func_init, NULL, {func_init, object});
    block->append_after(alloca, init);

    // store value to object
    auto *store = new Store(object, new Constant(codegen::HeaderLayoutOffsets::FieldOffset, UINT64), value);
    block->append_after(init, store);

    // object is ready
    return object;
}

Instruction *Unboxing::load_primitive(Operand *base) const
{
    // prepare an operand instead of the object for primitive
    auto *value = new Operand(INT64);
    auto *offset = new Constant(codegen::HeaderLayoutOffsets::FieldOffset, UINT64);
    return new Load(value, base, offset);
}

void Unboxing::replace_uses(Operand *src, Instruction *dst_inst, std::stack<TypeLink> &replace) const
{
    std::vector<Instruction *> for_uses_update;

    // update uses of this INTEGER or BOOLEAN with a new operand
    for (auto *use : src->uses())
    {
        if (use == dst_inst)
        {
            continue; // already processed
        }

        if (!Instruction::isa<Call>(use))
        {
            for_uses_update.push_back(use);
        }
    }

    for (auto *inst : for_uses_update)
    {
        inst->update_use(src, dst_inst->def());
        replace.push({inst, src->type()});
    }
}

void Unboxing::prepare_args(std::vector<bool> &processed) const
{
    std::stack<TypeLink> replace;

    auto *entry = _cfg->root();

    for (auto *param : _func->params())
    {
        if (param->type() == INTEGER || param->type() == BOOLEAN)
        {
            if (!need_unboxing(param))
            {
                continue;
            }

            auto *load = load_primitive(param);
            entry->append_front(load);

            replace_uses(param, load, replace);
        }
    }

    replace_uses(replace, processed);
}

void Unboxing::prepare_lets_and_calls(std::vector<bool> &processed) const
{
    std::stack<TypeLink> replace;

    for (auto *b : _cfg->traversal<CFG::REVERSE_POSTORDER>())
    {
        // cannot append instructions during pass over them in block
        std::vector<std::pair<Instruction *, Instruction *>> for_append;

        // 1. search for moves of INTEGER/BOOLEAN constants (initial moves);
        for (auto *inst : b->insts())
        {
            // check if we can and need unboxing of this def
            auto *def = inst->def();
            if (!def || def->type() != INTEGER && def->type() != BOOLEAN)
            {
                continue;
            }

            if (!need_unboxing(def))
            {
                continue;
            }

            if (Instruction::isa<Move>(inst))
            {
                auto *oper = inst->use(0);

                // search for initial moves
                if (!Operand::isa<GlobalConstant>(oper))
                {
                    continue;
                }

                auto *initializer = Operand::as<GlobalConstant>(oper);
                // found instruction : value <- global_const
                auto *value = initializer->field(codegen::HeaderLayoutOffsets::FieldOffset);
                // create a new move and use its def in all uses of the original move except calls
                auto *move = new Move(new Operand(value->type()), value);

                replace_uses(def, move, replace);
                for_append.push_back({inst, move});
            }
            // 2. search for calls with INTEGER/BOOLEAN return value
            else if (Instruction::isa<Call>(inst))
            {
                auto *call = Instruction::as<Call>(inst);
                assert(call->callee()->has_return());

                if (call->callee()->is_runtime())
                {
                    continue;
                }

                auto *def = call->def();
                auto *load = load_primitive(def);

                replace_uses(def, load, replace);
                for_append.push_back({inst, load});
            }
        }

        for (auto &[where, what] : for_append)
        {
            b->append_after(where, what);
        }
    }

    replace_uses(replace, processed);
}

void Unboxing::replace_uses(std::stack<TypeLink> &s, std::vector<bool> &processed) const
{
    while (!s.empty())
    {
        auto link = s.top();
        auto *inst = link._inst;
        s.pop();

        if (processed[inst->id()])
        {
            continue;
        }

        processed[inst->id()] = true;

        if (Instruction::isa<Load>(inst))
        {
            replace_load(Instruction::as<Load>(inst), link._type, s);
        }
        else if (Instruction::isa<Store>(inst))
        {
            replace_store(Instruction::as<Store>(inst), link._type, s);
        }
        else if (Instruction::isa<Call>(inst))
        {
            wrap_primitives(inst, link._type);
        }
        else if (Instruction::isa<Ret>(inst))
        {
            wrap_primitives(inst, link._type);
        }
        else
        {
            if (!inst->def())
            {
                continue;
            }

            // TODO: INT64 for now. FIXME!
            inst->def()->set_type(INT64);

            for (auto *use : inst->def()->uses())
            {
                s.push({use, link._type});
            }
        }
    }
}

void Unboxing::replace_load(Load *load, OperandType type, std::stack<TypeLink> &s) const
{
    // load from Integer object. Have to be replaced by moves
    auto *result = load->def();
    auto *object = load->use(0); // former object and now it's an INT64
    auto *offset = load->use(1);
    auto offset_val = Operand::as<Constant>(offset)->value();

    auto *block = load->holder();
    bool is_value_access = false; // propagate value of this load only for field accesses

    // don't change def. Will be eliminated by copy propagation
    Instruction *move = nullptr;

    // all acceses to Integer objects by constant offsets
    switch (offset_val)
    {
    case codegen::HeaderLayoutOffsets::FieldOffset: {
        move = new Move(result, object);
        is_value_access = true;
        break;
    }
    case codegen::HeaderLayoutOffsets::DispatchTableOffset: {
        // load of the dispatch table. We know exactyly the type
        move = new Move(result, _data.class_disp_tab(operand_to_klass(type)));
        break;
    }
    default:
        SHOULD_NOT_REACH_HERE();
    }

    block->append_instead(load, move);

    if (is_value_access)
    {
        for (auto *use : result->uses())
        {
            s.push({use, type});
        }
    }
}

void Unboxing::wrap_primitives(Instruction *inst, OperandType type) const
{
    std::vector<Operand *> for_replace;

    for (auto *use : inst->uses())
    {
        // integer and boolean constants were replaced with INT64 values
        // TODO: INT64 for now. FIXME!
        if (use->type() == INT64)
        {
            for_replace.push_back(use);
        }
    }

    // noew create allocations and replace uses
    for (auto *value : for_replace)
    {
        auto *object = allocate_primitive(inst, value, operand_to_klass(type)->klass());
        inst->update_use(value, object);
    }
}

void Unboxing::replace_store(Store *store, OperandType type, std::stack<TypeLink> &s) const
{
    // store to/of Integer/Boolean object
    auto *object = store->use(0);
    auto *offset = store->use(1);
    auto *value = store->use(2);

    // Store of the recently unboxed value as a field.
    // TODO: INT64 for now. FIXME!
    if (object->type() != INTEGER && object->type() != BOOLEAN)
    {
        wrap_primitives(store, type);
        return;
    }

    // Replace use of newly allocated object with a new def
    // Access can be only to value field
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
                // object escapes to call that is not init. We can't delete allocation
                for_delete.clear();
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
        s.push({inst, type});
    }

    // We can delete object allocaion and initialization
    if (!for_delete.empty())
    {
        for_delete.push_back(object->def());
        Block::erase(for_delete);
    }
}