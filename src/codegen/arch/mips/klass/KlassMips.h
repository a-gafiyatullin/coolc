#pragma once

#include "codegen/constants/Constants.h"
#include "codegen/klass/Klass.h"
#include <string_view>

namespace codegen
{
class KlassMips : public Klass
{
  public:
    /*
     * Header:
     * 1) Tag
     * 2) Size
     * 3) Dispatch Table
     */
    static constexpr int HEADER_FIELDS = 3;

    static constexpr char FULL_METHOD_DELIM = '.';
    static constexpr std::string_view INIT_METHOD_NAME_SUFFIX = "_init";
    static constexpr std::string_view PROTOTYPE_NAME_SUFFIX = "_protObj";
    static constexpr std::string_view DISP_TAB_NAME_SUFFIX = "_dispTab";

    KlassMips(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder) : Klass(klass, builder) {}

    KlassMips() : Klass() {}

    size_t size() const override { return (_fields.size() + HEADER_FIELDS) * WORD_SIZE; }

    std::string method_full_name(const std::string &method_name) const override;

    size_t field_offset(const int &field_num) const override
    {
        GUARANTEE_DEBUG(field_num < _fields.size());
        return (field_num + HEADER_FIELDS) * WORD_SIZE;
    }

    /**
     * @brief Number of methods
     *
     * @return Number of methods
     */
    inline size_t methods_num() const { return _methods.size(); }

    /**
     * @brief Construct full name of the method for this Class
     *
     * @param n Method's number
     * @return Full name of the method
     */
    std::string method_full_name(const int &n) const;

    std::string init_method() const override { return name() + static_cast<std::string>(INIT_METHOD_NAME_SUFFIX); }

    std::string prototype() const override { return name() + static_cast<std::string>(PROTOTYPE_NAME_SUFFIX); }

    std::string disp_tab() const override { return name() + static_cast<std::string>(DISP_TAB_NAME_SUFFIX); }
};

class KlassBuilderMips : public KlassBuilder
{
  private:
    std::shared_ptr<Klass> make_klass(const std::shared_ptr<ast::Class> &klass) override;

  public:
    explicit KlassBuilderMips(const std::shared_ptr<semant::ClassNode> &root) : KlassBuilder(root) {}
};

}; // namespace codegen
