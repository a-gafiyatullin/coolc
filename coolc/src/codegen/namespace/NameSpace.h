#pragma once

#include "codegen/classes/Klass.h"

namespace codegen
{
/**
 * @brief NameSpace manages various name spaces
 */
class NameSpace
{
  private:
    static constexpr std::string_view DISP_TAB_SUFFIX = "_dispTab";
    static constexpr std::string_view PROTOTYPE_SUFFIX = "_protObj";
    static constexpr std::string_view INIT_SUFFIX = "_init";

    // labels
    static constexpr std::string_view TRUE_BRANCH_LABEL = "true_";
    static int TrueBranchLabelNum;

    static constexpr std::string_view FALSE_BRANCH_LABEL = "false_";
    static int FalseBranchLabelNum;

    static constexpr std::string_view MERGE_BLOCK_LABEL = "merge_";
    static int MergeBlockLabelNum;

    static constexpr std::string_view LOOP_HEADER_LABEL = "loop_header_";
    static int LoopHeaderLabelNum;

    static constexpr std::string_view LOOP_TAIL_LABEL = "loop_tail_";
    static int LoopTailLabelNum;

  public:
    /**
     * @brief Get the dispatch table name
     *
     * @param klass Klass handle
     * @return Dispatch table name
     */
    inline static std::string disp_table(const std::shared_ptr<Klass> &klass)
    {
        return klass->name() + static_cast<std::string>(DISP_TAB_SUFFIX);
    }

    /**
     * @brief Get the prototype name
     *
     * @param klass Klass handle
     * @return Prototype name
     */
    inline static std::string prototype(const std::shared_ptr<Klass> &klass)
    {
        return klass->name() + static_cast<std::string>(PROTOTYPE_SUFFIX);
    }

    /**
     * @brief Get the init method name
     *
     * @param klass Klass handle
     * @return Init method name
     */
    inline static std::string init_method(const std::shared_ptr<Klass> &klass)
    {
        return klass->name() + static_cast<std::string>(INIT_SUFFIX);
    }

    /**
     * @brief Get free true branch label name
     *
     * @return True branch label name
     */
    inline static std::string true_branch()
    {
        return static_cast<std::string>(TRUE_BRANCH_LABEL) + std::to_string(TrueBranchLabelNum++);
    }

    /**
     * @brief Get free false branch label name
     *
     * @return False branch label name
     */
    inline static std::string false_branch()
    {
        return static_cast<std::string>(FALSE_BRANCH_LABEL) + std::to_string(FalseBranchLabelNum++);
    }

    /**
     * @brief Get free merge block label name
     *
     * @return Merge block label name
     */
    inline static std::string merge_block()
    {
        return static_cast<std::string>(MERGE_BLOCK_LABEL) + std::to_string(MergeBlockLabelNum++);
    }

    /**
     * @brief Get free loop header label name
     *
     * @return Loop header label name
     */
    inline static std::string loop_header()
    {
        return static_cast<std::string>(LOOP_HEADER_LABEL) + std::to_string(LoopHeaderLabelNum++);
    }

    /**
     * @brief Get free loop tail label name
     *
     * @return Loop tail label name
     */
    inline static std::string loop_tail()
    {
        return static_cast<std::string>(LOOP_TAIL_LABEL) + std::to_string(LoopTailLabelNum++);
    }
};
}; // namespace codegen