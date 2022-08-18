#include "NameConstructor.h"

using namespace codegen;

const std::pair<std::string, bool> Names::COMMENT_NAMES[CommentsNumber] = {
    {"_type", true},

    {"entry_block", false},  {"true_block_", false}, {"false_block_", false},  {"merge_block_", false},
    {"loop_header_", false}, {"loop_body_", false},  {"loop_tail_", false},

    {"_char_str", true},     {"bool_const_", false}, {"int_const_", false},    {"str_const_", false},

    {"call_", false},        {"local_", false},      {"sub_", false},          {"add_", false},
    {"mul_", false},         {"div_", false},        {"slt_", false},          {"sgt_", false},
    {"sle_", false},         {"eq_", false},         {"or_", false},           {"phi_", false},
    {"xor_", false},         {"neg_", false},        {"not_", false},          {"not_null_", false},

    {"obj_tag_", false},     {"obj_size_", false},   {"obj_disp_tab_", false},

    {"val_", false},         {"stack_slot_", false}};

int Names::CommentNumber[CommentsNumber] = {};