#pragma once

#include "codegen/symnames/NameConstructor.h"
#include "semant/Semant.h"

namespace codegen
{

class KlassBuilder;

/**
 * @brief Klass represents Cool class
 *
 */
class Klass
{
    friend class KlassBuilder;

  protected:
    const std::shared_ptr<ast::Type> _klass;
    const std::shared_ptr<Klass> _parent_klass;

    // tags necessary for runtime and expressions constructions
    int _tag;
    int _child_max_tag;

    // CHA
    bool _is_leaf;

    // All fields and methods of this class
    std::vector<std::shared_ptr<ast::Feature>> _fields;
    std::vector<std::pair<std::shared_ptr<ast::Type>, std::shared_ptr<ast::Feature>>> _methods;

    void divide_features(const std::vector<std::shared_ptr<ast::Feature>> &features);

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
    Klass(const std::shared_ptr<ast::Class> &klass, const KlassBuilder *builder);

    /**
     * @brief Construct new empty Klass
     */
    Klass();

    /**
     * @brief Class name
     *
     * @return Class name
     */
    inline const std::string &name() const { return _klass->_string; }

    /**
     * @brief Parent class
     *
     * @return Parent class Klass
     */
    inline const std::shared_ptr<Klass> &parent() const { return _parent_klass; }

    /**
     * @brief Class type
     *
     * @return Class type
     */
    inline const std::shared_ptr<ast::Type> &klass() const { return _klass; }

    /**
     * @brief Size of the object of this class
     *
     * @return Size in bytes
     */
    virtual size_t size() const = 0;

    // ------------------------------------ FIELDS ------------------------------------

    /**
     * @brief Iterator to the first field
     *
     * @return Iterator to the first field
     */
    inline std::vector<std::shared_ptr<ast::Feature>>::const_iterator fields_begin() const { return _fields.begin(); }

    /**
     * @brief Iterator to the behind of the last field
     *
     * @return Iterator to the behind of the last field
     */
    inline std::vector<std::shared_ptr<ast::Feature>>::const_iterator fields_end() const { return _fields.end(); }

    /**
     * @brief Get field offset
     *
     * @param field_num Field number
     * @return Offset
     */
    virtual size_t field_offset(const int &field_num) const = 0;

    /**
     * @brief Fields number without header
     *
     * @return Fields number
     */
    int fields_num() const { return _fields.size(); }
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
     * @brief Get index in dispatch table for the given method
     *
     * @param method_name Name of the method
     * @return Index
     */
    size_t method_index(const std::string &method_name) const;

    /**
     * @brief Construct full name of the method for this Class
     *
     * @param method_name Name of the method
     * @return Full name of the method
     */
    virtual std::string method_full_name(const std::string &method_name) const = 0;

    /**
     * @brief Get the init method name
     *
     * @param klass Class name
     * @return Init method name
     */
    virtual std::string init_method() const = 0;

    /**
     * @brief Get the prototype name
     *
     * @param klass Class name
     * @return Prototype name
     */
    virtual std::string prototype() const = 0;

    /**
     * @brief Get the dispatch table name
     *
     * @param klass Class name
     * @return Dispatch table name
     */
    virtual std::string disp_tab() const = 0;
    // ------------------------------------ TAGS ------------------------------------

    /**
     * @brief Class tag
     * @return Class tag
     */
    inline int tag() const { return _tag; }

    /**
     * @brief The biggest tag among all children of this class
     *
     * @return Class tag
     */
    inline int child_max_tag() const { return _child_max_tag; }

    inline bool is_leaf() const { return _is_leaf; }

    virtual ~Klass() {}
};

/**
 * @brief KlassBuilder builds Klasses for Cool classes
 *
 */
class KlassBuilder
{
    friend class Klass;

  protected:
    // Classes from semant
    const std::shared_ptr<semant::ClassNode> _root;

    // Map of classes for inheritance
    std::unordered_map<std::string, std::shared_ptr<Klass>> _klasses;

    // Klasses sorted by tag
    std::vector<std::shared_ptr<Klass>> _klasses_by_tag;

    int build_klass(const std::shared_ptr<semant::ClassNode> &node, const int &tag);

    /**
     * @brief Create a Klass object. Pass nullptr for the first parameter to create initial empty Klass (head of
     * hierarchy)
     *
     * @param klass ast::Class representation of Cool Class
     * @return New Klass object
     */
    virtual std::shared_ptr<Klass> make_klass(const std::shared_ptr<ast::Class> &klass) = 0;

  public:
    /**
     * @brief Build map of Klasses for Cool classes
     *
     * @param root Root of the class hierarchy
     */
    explicit KlassBuilder(const std::shared_ptr<semant::ClassNode> &root);

    /**
     * @brief Initialize KlassBuilder
     *
     */
    void init();

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
     * @brief Klass that represents Cool Class
     *
     * @param class_name Class name
     * @return Klass instance
     */
    inline const std::shared_ptr<Klass> &klass(const std::string &class_name) const
    {
        GUARANTEE_DEBUG(_klasses.find(class_name) != _klasses.end());
        return _klasses.at(class_name);
    }

    /**
     * @brief Klasses vector sorted by tag
     *
     * @return Vector of Klasses
     */
    inline const std::vector<std::shared_ptr<Klass>> &klasses() const { return _klasses_by_tag; }

    /**
     * @brief Get root of the Class hierarchy
     *
     * @return Root of the Class hierarchy
     */
    inline const std::shared_ptr<semant::ClassNode> &root() const { return _root; }
};

}; // namespace codegen
