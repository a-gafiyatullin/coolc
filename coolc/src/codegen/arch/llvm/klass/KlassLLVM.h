#pragma once

#include "codegen/arch/llvm/runtime/RuntimeLLVM.h"
#include "codegen/klass/Klass.h"

namespace codegen
{
class KlassLLVM : public Klass
{
  public:
    static constexpr char FULL_METHOD_DELIM = '_';

    KlassLLVM(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder) : Klass(klass, builder)
    {
    }

    KlassLLVM() : Klass()
    {
    }

    size_t size() const override
    {
        return (_fields.size()) * WORD_SIZE + HeaderLayoutSizes::HeaderSize;
    }

    std::string method_full_name(const std::string &method_name) const override;

    size_t field_offset(const int &field_num) const override
    {
        GUARANTEE_DEBUG(field_num < _fields.size());
        return field_num + HeaderLayout::HeaderLayoutElemets;
    }

    /**
     * @brief Field type by absolute index
     *
     * @param field_idx Absolute index
     * @return Field type
     */
    const std::shared_ptr<ast::Type> &field_type(const int &field_idx) const
    {
        return _fields[field_idx - HeaderLayoutSizes::HeaderSize]->_type;
    }
};

class KlassBuilderLLVM : public KlassBuilder
{
  private:
    std::shared_ptr<Klass> make_klass(const std::shared_ptr<ast::Class> &klass) override;

  public:
    explicit KlassBuilderLLVM(const std::shared_ptr<semant::ClassNode> &root) : KlassBuilder(root)
    {
    }
};

}; // namespace codegen