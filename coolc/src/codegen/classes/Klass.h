#pragma once

#include "semant/Semant.h"
#include <cstddef>
#include <cstdint>

namespace codegen
{

class KlassBuilder;

/**
 * @brief Klass represents COOL class
 *
 */
class Klass
{
    friend class KlassBuilder;

  public:
    /*
     * Header:
     * 0) Reference Counter (not MIPS)
     * 1) Tag
     * 2) Size
     * 3) Dispatch Table
     */

    static constexpr int HEADER_FIELDS = 4 MIPS_ONLY(-1);

  private:
    static constexpr std::string_view DISP_TAB_SUFFIX = "_dispTab";
    static constexpr std::string_view PROTOTYPE_SUFFIX = "_protObj";
    static constexpr std::string_view INIT_SUFFIX = "_init";

    const std::shared_ptr<ast::Type> _klass;

    // tags necessary for runtime and expressions constructions
    int _tag;
    int _child_max_tag;

    // All fields and methods of this class
    std::vector<std::shared_ptr<ast::Feature>> _fields;
    std::vector<std::pair<std::shared_ptr<ast::Type>, std::shared_ptr<ast::Feature>>> _methods;

    void divide_features(const std::vector<std::shared_ptr<ast::Feature>> &features);

    void init_header();

    inline void set_tags(const int &klass_tag, const int &child_max_tag)
    {
        _tag = klass_tag;
        _child_max_tag = child_max_tag;
    }

#ifdef CODEGEN_VERBOSE_ONLY
    void dump_fields() const;
    void dump_methods() const;
#endif // CODEGEN_VERBOSE_ONLY

  public:
    /**
     * @brief Construct new Klass
     * @param klass AST for this Class
     * @param builder KlassBuilder instance
     */
    Klass(const std::shared_ptr<ast::Class> &klass, const KlassBuilder &builder);

    /**
     * @brief Construct new empty Klass
     */
    Klass();

    /**
     * @brief Class name
     *
     * @return Class name
     */
    inline std::string name() const
    {
        return _klass->_string;
    }

    // ------------------------------------ FIELDS ------------------------------------

    /**
     * @brief Iterator to the first field
     *
     * @return Iterator to the first field
     */
    inline std::vector<std::shared_ptr<ast::Feature>>::const_iterator fields_begin() const
    {
        return _fields.begin() + HEADER_FIELDS;
    }

    /**
     * @brief Iterator to the behind of the last field
     *
     * @return Iterator to the behind of the last field
     */
    inline std::vector<std::shared_ptr<ast::Feature>>::const_iterator fields_end() const
    {
        return _fields.end();
    }

    /**
     * @brief Size of the object of this class
     *
     * @return Size in bytes
     */
    inline uint32_t size() const
    {
        return _fields.size() * WORD_SIZE;
    }

    /**
     * @brief Get field offset
     *
     * @param field_num Field number
     * @return Offset
     */
    inline uint32_t field_offset(const int &field_num) const
    {
        GUARANTEE_DEBUG(field_num < _fields.size());
        return (field_num + HEADER_FIELDS) * WORD_SIZE;
    }
    // ------------------------------------ METHODS ------------------------------------

    /**
     * @brief Iterator to the first method
     *
     * @return Iterator to the first method
     */
    inline std::vector<std::pair<std::shared_ptr<ast::Type>, std::shared_ptr<ast::Feature>>>::const_iterator
    methods_begin() const
    {
        return _methods.begin();
    }

    /**
     * @brief Iterator to the behind of the last method
     *
     * @return Iterator to the behind of the last method
     */
    inline std::vector<std::pair<std::shared_ptr<ast::Type>, std::shared_ptr<ast::Feature>>>::const_iterator
    methods_end() const
    {
        return _methods.end();
    }

    /**
     * @brief Number of methods
     *
     * @return Number of methods
     */
    inline size_t methods_num() const
    {
        return _methods.size();
    }

    /**
     * @brief Get offset in dispatch table for the given method
     *
     * @param method_name Name of the method
     * @return Offset in bytes
     */
    uint64_t method_offset(const std::string &method_name) const;

    /**
     * @brief Construct full name of the method for this Class
     *
     * @param method_name Name of the method
     * @return Full name of the method
     */
    std::string method_full_name(const std::string &method_name) const;

    /**
     * @brief Construct full name of the method for this Class
     *
     * @param n Method's number
     * @return Full name of the method
     */
    std::string method_full_name(const int &n) const;

    /**
     * @brief Get the init method name
     *
     * @return Init method name
     */
    inline std::string init_method() const
    {
        return _klass->_string + static_cast<std::string>(INIT_SUFFIX);
    }

    // ------------------------------------ TAGS ------------------------------------

    /**
     * @brief Class tag
     * @return Class tag
     */
    inline int tag() const
    {
        return _tag;
    }

    /**
     * @brief The biggest tag among all children of this class
     *
     * @return Class tag
     */
    inline int child_max_tag() const
    {
        return _child_max_tag;
    }

    // ------------------------------------ TABLES AND SYMBOLS ------------------------------------

    /**
     * @brief Get the dispatch table name
     *
     * @return Dispatch table name
     */
    inline std::string disp_table() const
    {
        return _klass->_string + static_cast<std::string>(DISP_TAB_SUFFIX);
    }

    /**
     * @brief Get the prototype name
     *
     * @return Prototype name
     */
    inline std::string prototype() const
    {
        return _klass->_string + static_cast<std::string>(PROTOTYPE_SUFFIX);
    }
};

/**
 * @brief KlassBuilder builds Klasses for COOL classes
 *
 */
class KlassBuilder
{
    friend class Klass;

  private:
    // map of classes for inheritance
    std::unordered_map<std::string, std::shared_ptr<Klass>> _klasses;

    // Klasses sorted by tag
    std::vector<std::shared_ptr<Klass>> _klasses_by_tag;

    int build_klass(const std::shared_ptr<semant::ClassNode> &node, const int &tag);

  public:
    /**
     * @brief Build map of Klasses for COOL classes
     *
     * @param root Root of the class hierarhy
     */
    KlassBuilder(const std::shared_ptr<semant::ClassNode> &root);

    /**
     * @brief Class tag
     *
     * @param class_name Class name
     * @return Class tag
     */
    inline int tag(const std::string &class_name) const
    {
        GUARANTEE_DEBUG(_klasses.find(class_name) != _klasses.end());
        return _klasses.at(class_name)->tag();
    }

    /**
     * @brief Iterator to the first Klass
     *
     * @return Iterator to the first Klass
     */
    inline std::unordered_map<std::string, std::shared_ptr<Klass>>::const_iterator begin() const
    {
        return _klasses.begin();
    }

    /**
     * @brief Iterator to the behind of the last Klass
     *
     * @return Iterator to the behind of the last Klass
     */
    inline std::unordered_map<std::string, std::shared_ptr<Klass>>::const_iterator end() const
    {
        return _klasses.end();
    }

    /**
     * @brief Klass that represents Class
     *
     * @param class_name Class name
     * @return Klass instance
     */
    inline std::shared_ptr<Klass> klass(const std::string &class_name) const
    {
        GUARANTEE_DEBUG(_klasses.find(class_name) != _klasses.end());
        return _klasses.at(class_name);
    }

    /**
     * @brief Klasses vector sorted by tag
     *
     * @return Vector of Klasses
     */
    inline const std::vector<std::shared_ptr<Klass>> &klasses() const
    {
        return _klasses_by_tag;
    }
};

}; // namespace codegen
