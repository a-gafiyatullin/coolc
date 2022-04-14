#pragma once

#include "codegen/constants/Constants.h"
#include "codegen/klass/Klass.h"

namespace codegen
{
class KlassLLVM : public Klass
{
  public:
    /*
     * Header:
     * 1) Mark
     * 2) Tag
     * 3) Size
     * 4) Dispatch Table
     */
    static constexpr int HEADER_FIELDS = 4;

    static constexpr char FULL_METHOD_DELIM = '_';

    KlassLLVM(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder) : Klass(klass, builder)
    {
    }

    KlassLLVM() : Klass()
    {
    }

    size_t size() const override
    {
        return (_fields.size() + HEADER_FIELDS) * WORD_SIZE;
    }

    std::string method_full_name(const std::string &method_name) const override;

    size_t field_offset(const int &field_num) const override
    {
        GUARANTEE_DEBUG(field_num < _fields.size());
        return field_num + HEADER_FIELDS;
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