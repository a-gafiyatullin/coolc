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
        TYPE,

        ENTRY_BLOCK,
        TRUE_BRANCH,
        FALSE_BRANCH,
        MERGE_BLOCK,
        LOOP_HEADER,
        LOOP_BODY,
        LOOP_TAIL,

        CHAR_STRING,
        CONST_BOOL,
        CONST_INT,
        CONST_STRING,

        CALL,
        ALLOCA,
        SUB,
        ADD,
        MUL,
        DIV,
        CMP_SLT,
        CMP_SGT,
        CMP_SLE,
        CMP_EQ,
        OR,
        PHI,
        XOR,
        NEG,
        NOT,

        OBJ_TAG,
        OBJ_SIZE,
        OBJ_DISP_TAB,

        VALUE,

        CommentsNumber
    };

  private:
    static const std::pair<std::string, bool> COMMENT_NAMES[CommentsNumber];
    static int CommentNumber[CommentsNumber];

  public:
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