#pragma once

#include "codegen/arch/myir/runtime/RuntimeMyIR.hpp"
#include "codegen/klass/Klass.h"

namespace codegen
{
class KlassMyIR : public Klass
{
  public:
    static constexpr char FULL_METHOD_DELIM = '_';
    static constexpr std::string_view INIT_METHOD_SUFFIX = "-init";
    static constexpr std::string_view PROTOTYPE_NAME_SUFFIX = "-protObj";
    static constexpr std::string_view DISP_TAB_NAME_SUFFIX = "_dispTab";

    KlassMyIR(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder) : Klass(klass, builder) {}

    KlassMyIR() : Klass() {}

    size_t size() const override { return (_fields.size()) * WORD_SIZE + HeaderLayoutSizes::HeaderSize; }

    std::string method_full_name(const std::string &method_name) const override;

    size_t field_offset(const int &field_num) const override
    {
        GUARANTEE_DEBUG(field_num < _fields.size());
        return field_num * WORD_SIZE + HeaderLayoutSizes::HeaderSize;
    }

    /**
     * @brief Field type by absolute index
     *
     * @param field_idx Absolute index
     * @return Field type
     */
    const std::shared_ptr<ast::Type> &field_type(const int &field_idx) const
    {
        GUARANTEE_DEBUG(field_idx - HeaderLayout::HeaderLayoutElemets < _fields.size());
        return _fields[field_idx - HeaderLayout::HeaderLayoutElemets]->_type;
    }

    std::string init_method() const override { return name() + static_cast<std::string>(INIT_METHOD_SUFFIX); }

    std::string prototype() const override { return name() + static_cast<std::string>(PROTOTYPE_NAME_SUFFIX); }

    std::string disp_tab() const override { return name() + static_cast<std::string>(DISP_TAB_NAME_SUFFIX); }
};

class KlassBuilderMyIR : public KlassBuilder
{
  private:
    std::shared_ptr<Klass> make_klass(const std::shared_ptr<ast::Class> &klass) override;

  public:
    explicit KlassBuilderMyIR(const std::shared_ptr<semant::ClassNode> &root) : KlassBuilder(root) {}
};

}; // namespace codegen