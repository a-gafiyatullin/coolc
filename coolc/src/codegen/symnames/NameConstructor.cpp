#include "NameConstructor.h"

using namespace codegen;

const std::pair<std::string, bool> Names::COMMENT_NAMES[CommentsNumber] = {
    {"_dispTab", true},      {"_protObj", true},      {"_init", true},       {"_type", true},

    {"true_", false},        {"false_", false},       {"merge_", false},     {"loop_header_", false},
    {"loop_tail_", false},   {"call_", false},        {"gep_", false},       {"load_", false},
    {"cast_to_", false},     {"_char_str", true},

    {"bool_const_", false},  {"int_const_", false},   {"str_const_", false},

    {"entry_block", false},

    {"sub_res_", false},     {"add_res_", false},     {"mul_res_", false},   {"div_res_", false},
    {"cmp_slt_res_", false}, {"cmp_sle_res_", false}, {"phi_res_", false}};

int Names::CommentNumber[CommentsNumber] = {};