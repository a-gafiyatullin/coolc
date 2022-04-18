#pragma once

#include <string>

namespace codegen
{
/**
 * @brief Names manages various names
 */
class Names
{
  public:
    enum Comment
    {
        DISP_TAB,
        PROTOTYPE,
        INIT_METHOD,
        TYPE,
        TRUE_BRANCH,
        FALSE_BRANCH,
        MERGE_BLOCK,
        LOOP_HEADER,
        LOOP_TAIL,
        CALL,
        GEP,
        LOAD,
        CAST,
        CHAR_STRING,
        CONST_BOOL,
        CONST_INT,
        CONST_STRING,
        ENTRY_BLOCK,
        SUB,
        ADD,
        MUL,
        DIV,
        CMP_SLT,
        CMP_SLE,
        CMP_EQ,
        PHI,

        CommentsNumber
    };

  private:
    static const std::pair<std::string, bool> COMMENT_NAMES[CommentsNumber];
    static int CommentNumber[CommentsNumber];

  public:
    /**
     * @brief Get the dispatch table name
     *
     * @param klass Class name
     * @return Dispatch table name
     */
    inline static std::string disp_table(const std::string &klass)
    {
        return name(DISP_TAB, klass);
    }

    /**
     * @brief Get the prototype name
     *
     * @param klass Class name
     * @return Prototype name
     */
    inline static std::string prototype(const std::string &klass)
    {
        return name(PROTOTYPE, klass);
    }

    /**
     * @brief Get the init method name
     *
     * @param klass Class name
     * @return Init method name
     */
    inline static std::string init_method(const std::string &klass)
    {
        return name(INIT_METHOD, klass);
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

    /**
     * @brief Get free integer constant name
     *
     * @return Integer constant name
     */
    inline static std::string int_constant()
    {
        return name(CONST_INT);
    }

    /**
     * @brief Get free bool constant name
     *
     * @return Bool constant name
     */
    inline static std::string bool_constant()
    {
        CommentNumber[CONST_BOOL] %= 2;
        return name(CONST_BOOL);
    }

    /**
     * @brief Get free string constant name
     *
     * @return String constant name
     */
    inline static std::string string_constant()
    {
        return name(CONST_STRING);
    }

    /**
     * @brief Get name for string
     *
     * @param type Name type
     * @param str String
     * @return Name for string
     */
    inline static std::string name(const Comment &type, const std::string &str)
    {
        if (COMMENT_NAMES[type].second)
        {
            return str + COMMENT_NAMES[type].first;
        }

        return COMMENT_NAMES[type].first + str;
    }

    /**
     * @brief Get new name of the specified type
     *
     * @param type Name type
     * @param next Do next
     * @return Name of the specified type
     */
    inline static std::string name(const Comment &type)
    {
        if (COMMENT_NAMES[type].second)
        {
            return std::to_string((CommentNumber[type]++)) + COMMENT_NAMES[type].first;
        }

        return COMMENT_NAMES[type].first + std::to_string((CommentNumber[type]++));
    }

    /**
     * @brief Get raw name of the specified type
     *
     * @param type Name type
     * @return Raw Name of the specified type
     */
    inline static std::string comment(const Comment &type)
    {
        return COMMENT_NAMES[type].first;
    }
};
}; // namespace codegen