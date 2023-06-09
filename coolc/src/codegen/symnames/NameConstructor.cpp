#include "NameConstructor.h"

using namespace codegen;

const std::pair<std::string, bool> Names::COMMENT_NAMES[CommentsNumber] = {
    {"_type", true},

    {"entry_block", false},  {"true_block_", false}, {"false_block_", false}, {"merge_block_", false},
    {"loop_header_", false}, {"loop_body_", false},  {"loop_tail_", false},

    {"bool_const_", false},  {"int_const_", false},  {"str_const_", false}};

int Names::CommentNumber[CommentsNumber] = {};

void Names::reset()
{
    static int RESET_IDX[] = {ENTRY_BLOCK, TRUE_BRANCH, FALSE_BRANCH, MERGE_BLOCK, LOOP_HEADER, LOOP_BODY, LOOP_TAIL};
    for (auto idx : RESET_IDX)
    {
        CommentNumber[idx] = 0;
    }
}