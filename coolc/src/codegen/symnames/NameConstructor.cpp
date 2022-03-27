#include "NameConstructor.h"

using namespace codegen;

int NameConstructor::TrueBranchLabelNum = 0;
int NameConstructor::FalseBranchLabelNum = 0;
int NameConstructor::MergeBlockLabelNum = 0;
int NameConstructor::LoopHeaderLabelNum = 0;
int NameConstructor::LoopTailLabelNum = 0;
int NameConstructor::IntegerNum = 0;
int NameConstructor::BooleanNum = 0;
int NameConstructor::StringNum = 0;