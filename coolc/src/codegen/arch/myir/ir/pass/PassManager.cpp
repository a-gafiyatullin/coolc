#include "PassManager.hpp"
#include "codegen/arch/myir/ir/IR.inline.hpp"

using namespace myir;

void PassManager::PassManager::run()
{
    for (auto &[name, func] : _module.get<Function>())
    {
        for (auto *pass : _passes)
        {
            pass->run(func);
        }
    }
}

PassManager::~PassManager()
{
    for (auto *pass : _passes)
    {
        delete pass;
    }
}