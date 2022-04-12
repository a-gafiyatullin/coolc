#pragma once

#include <string>

namespace codegen
{
/**
 * @brief NameConstructor manages various name spaces
 */
class NameConstructor
{
  private:
    static int TrueBranchLabelNum;
    static int FalseBranchLabelNum;
    static int MergeBlockLabelNum;
    static int LoopHeaderLabelNum;
    static int LoopTailLabelNum;
    static int IntegerNum;
    static int BooleanNum;
    static int StringNum;

  public:
    /**
     * @brief Suffices for prototypes and init methods construction
     *
     */
    static constexpr std::string_view DISP_TAB_SUFFIX = "_dispTab";
    static constexpr std::string_view PROTOTYPE_SUFFIX = "_protObj";
    static constexpr std::string_view INIT_SUFFIX = "_init";

    /**
     * @brief Prefixes for code generation names construction
     *
     */
    static constexpr std::string_view TRUE_BRANCH_PREFIX = "true_";
    static constexpr std::string_view FALSE_BRANCH_PREFIX = "false_";
    static constexpr std::string_view MERGE_BLOCK_PREFIX = "merge_";
    static constexpr std::string_view LOOP_HEADER_PREFIX = "loop_header_";
    static constexpr std::string_view LOOP_TAIL_PREFIX = "loop_tail_";

    /**
     * @brief Prefixes for constants
     *
     */
    static constexpr std::string_view CONST_BOOL_PREFIX = "bool_const_";
    static constexpr std::string_view CONST_INT_PREFIX = "int_const_";
    static constexpr std::string_view CONST_STRING_PREFIX = "str_const_";

    /**
     * @brief Names for code generation
     *
     */
    static constexpr std::string_view ENTRY_BLOCK_NAME = "entry_block";
    static constexpr std::string_view CALL_PREFIX = "call_";
    static constexpr std::string_view GEP_PREFIX = "gep_";

    /**
     * @brief Get the dispatch table name
     *
     * @param klass Class name
     * @return Dispatch table name
     */
    inline static std::string disp_table(const std::string &klass)
    {
        return klass + static_cast<std::string>(DISP_TAB_SUFFIX);
    }

    /**
     * @brief Get the prototype name
     *
     * @param klass Class name
     * @return Prototype name
     */
    inline static std::string prototype(const std::string &klass)
    {
        return klass + static_cast<std::string>(PROTOTYPE_SUFFIX);
    }

    /**
     * @brief Get the init method name
     *
     * @param klass Class name
     * @return Init method name
     */
    inline static std::string init_method(const std::string &klass)
    {
        return klass + static_cast<std::string>(INIT_SUFFIX);
    }

    /**
     * @brief Get free integer constant name
     *
     * @return Integer constant name
     */
    inline static std::string int_constant()
    {
        return static_cast<std::string>(CONST_INT_PREFIX) + std::to_string(IntegerNum++);
    }

    /**
     * @brief Get free bool constant name
     *
     * @return Bool constant name
     */
    inline static std::string bool_constant()
    {
        BooleanNum = BooleanNum % 2;
        return static_cast<std::string>(CONST_BOOL_PREFIX) + std::to_string(BooleanNum++);
    }

    /**
     * @brief Get free string constant name
     *
     * @return String constant name
     */
    inline static std::string string_constant()
    {
        return static_cast<std::string>(CONST_STRING_PREFIX) + std::to_string(StringNum++);
    }

    /**
     * @brief Construct full method name
     *
     * @param klass Class name
     * @param method Method name
     * @param delim Delimiter
     * @return Full method name
     */
    inline static std::string method_full_name(const std::string &klass, const std::string &method, const char &delim)
    {
        return klass + delim + method;
    }

    // -------------------------------------- CODEGEN SUPPORT --------------------------------------
    /**
     * @brief Get free true branch label name
     *
     * @return True branch label name
     */
    inline static std::string true_branch()
    {
        return static_cast<std::string>(TRUE_BRANCH_PREFIX) + std::to_string(TrueBranchLabelNum++);
    }

    /**
     * @brief Get free false branch label name
     *
     * @return False branch label name
     */
    inline static std::string false_branch()
    {
        return static_cast<std::string>(FALSE_BRANCH_PREFIX) + std::to_string(FalseBranchLabelNum++);
    }

    /**
     * @brief Get free merge block label name
     *
     * @return Merge block label name
     */
    inline static std::string merge_block()
    {
        return static_cast<std::string>(MERGE_BLOCK_PREFIX) + std::to_string(MergeBlockLabelNum++);
    }

    /**
     * @brief Get free loop header label name
     *
     * @return Loop header label name
     */
    inline static std::string loop_header()
    {
        return static_cast<std::string>(LOOP_HEADER_PREFIX) + std::to_string(LoopHeaderLabelNum++);
    }

    /**
     * @brief Get free loop tail label name
     *
     * @return Loop tail label name
     */
    inline static std::string loop_tail()
    {
        return static_cast<std::string>(LOOP_TAIL_PREFIX) + std::to_string(LoopTailLabelNum++);
    }

    /**
     * @brief Get comment for call intsruction
     *
     * @param method Method to call
     * @return String comment
     */
    inline static std::string call(const std::string &method)
    {
        return static_cast<std::string>(CALL_PREFIX) + method;
    }

    /**
     * @brief Get comment for get element pointer intsruction
     *
     * @param method Object to get element pointer
     * @return String comment
     */
    inline static std::string gep(const std::string &object)
    {
        return static_cast<std::string>(GEP_PREFIX) + object;
    }
};
}; // namespace codegen