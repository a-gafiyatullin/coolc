#include "RuntimeMyIR.hpp"
#include "codegen/arch/myir/klass/KlassMyIR.hpp"

using namespace codegen;

RuntimeMethod::RuntimeMethod(myir::Module &module, const std::string &name, myir::OperandType ret,
                             const std::vector<myir::oper> &args, bool need_gc, RuntimeMyIR &runtime)
    : _func(std::make_shared<myir::Function>(name, args, ret))
{
    runtime._symbol_by_name.insert({name, this});
    module.add_function(_func);

    // set gc
    if (!need_gc)
    {
        _func->set_is_leaf();
    }
}

RuntimeMyIR::RuntimeMyIR(myir::Module &module)
    : _equals(module, SYMBOLS[RuntimeMyIRSymbols::EQUALS], myir::OperandType::INT32,
              {std::make_shared<myir::Operand>("lhs", myir::OperandType::POINTER),
               std::make_shared<myir::Operand>("rhs", myir::OperandType::POINTER)},
              false, *this),

      _gc_alloc(module, SYMBOLS[RuntimeMyIRSymbols::GC_ALLOC], myir::OperandType::POINTER,
                {std::make_shared<myir::Operand>("tag", myir::OperandType::INT32),
                 std::make_shared<myir::Operand>("size", myir::OperandType::UINT64),
                 std::make_shared<myir::Operand>("dt", myir::OperandType::POINTER)},
                true, *this),

      _case_abort(module, SYMBOLS[RuntimeMyIRSymbols::CASE_ABORT], myir::OperandType::VOID,
                  {std::make_shared<myir::Operand>("tag", myir::OperandType::INT32)}, false, *this),

      _dispatch_abort(module, SYMBOLS[RuntimeMyIRSymbols::DISPATCH_ABORT], myir::OperandType::VOID,
                      {std::make_shared<myir::Operand>("filename", myir::OperandType::POINTER),
                       std::make_shared<myir::Operand>("linenumber", myir::OperandType::INT32)},
                      false, *this),

      _case_abort_2(module, SYMBOLS[RuntimeMyIRSymbols::CASE_ABORT_2], myir::OperandType::VOID,
                    {std::make_shared<myir::Operand>("filename", myir::OperandType::POINTER),
                     std::make_shared<myir::Operand>("linenumber", myir::OperandType::INT32)},
                    false, *this),

      _init_runtime(module, SYMBOLS[RuntimeMyIRSymbols::INIT_RUNTIME], myir::OperandType::VOID,
                    {std::make_shared<myir::Operand>("argc", myir::OperandType::INT32),
                     std::make_shared<myir::Operand>("argv", myir::OperandType::POINTER)},
                    false, *this),

      _finish_runtime(module, SYMBOLS[RuntimeMyIRSymbols::FINISH_RUNTIME], myir::OperandType::VOID, {}, false, *this)

#ifdef DEBUG
      ,
      _verify_oop(module, SYMBOLS[RuntimeMyIRSymbols::VERIFY_OOP], myir::OperandType::VOID,
                  {std::make_shared<myir::Operand>("oop", myir::OperandType::POINTER)}, false, *this)
#endif // DEBUG

      ,
      _stack_pointer(std::make_shared<myir::GlobalVariable>(SYMBOLS[RuntimeMyIRSymbols::STACK_POINTER],
                                                            std::vector<myir::oper>{}, myir::POINTER)),

      _frame_pointer(std::make_shared<myir::GlobalVariable>(SYMBOLS[RuntimeMyIRSymbols::FRAME_POINTER],
                                                            std::vector<myir::oper>{}, myir::POINTER))
{
    _header_layout_types[HeaderLayout::Mark] = myir::OperandType::UINT32;
    _header_layout_types[HeaderLayout::Tag] = myir::OperandType::UINT32;
    _header_layout_types[HeaderLayout::Size] = myir::OperandType::UINT64;
    _header_layout_types[HeaderLayout::DispatchTable] = myir::OperandType::POINTER;

    module.add_global_variable(_stack_pointer);
    module.add_global_variable(_frame_pointer);
}

const std::string RuntimeMyIR::SYMBOLS[RuntimeMyIRSymbolsSize] = {
    "_equals",         "_case_abort",    "_case_abort_2",   "_gc_alloc",
    "_dispatch_abort", "_init_runtime",  "_finish_runtime",

#ifdef DEBUG
    "_verify_oop",
#endif // DEBUG

    "class_nameTab",   "class_objTab",   "_int_tag",        "_bool_tag",
    "_string_tag",     "_stack_pointer", "_frame_pointer"};