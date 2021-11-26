#pragma once

#include <string>
#include <vector>
#include <set>
#include <cassert>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <regex>
#include "utils/Utils.h"

#ifdef CODEGEN_FULL_VERBOSE
#include "utils/logger/Logger.h"
#endif // CODEGEN_FULL_VERBOSE

namespace codegen
{
    class CodeBuffer
    {
    private:
        std::string _buffer;

    public:
        CodeBuffer() = default;
        CodeBuffer(const CodeBuffer &buffer) : _buffer(buffer._buffer) {}

        inline void save(const std::string &command) { _buffer += command + "\n"; }

        CodeBuffer &operator+=(const CodeBuffer &buffer);
        operator std::string() const { return _buffer; }
    };

    class Assembler;

    // not all mips regiters!
    class Register
    {
    public:
        enum REG
        {
            $sp,
            $fp,
            $ra,
            $t0,
            $t1,
            $t2,
            $t3,
            $t4,
            $t5,
            $t6,
            $a0,
            $a1,
            $s0,
            $zero
        };

        // registers are tracked in Assmebler
        Register(Assembler &assembler, const REG &reg);

        // delete register from Assembler
        ~Register();

        operator std::string() const { return _REG_TO_STR[_reg]; }
        inline static std::string get_reg_name(const Register::REG &reg) { return _REG_TO_STR[reg]; }

    private:
        static const std::vector<std::string> _REG_TO_STR;

        const REG _reg;
        Assembler &_asm;
    };

    class AssemblerMarkSection;

    class Label
    {
    private:
        const std::string _name;

    public:
        enum LabelPolicy
        {
            MUST_BIND,
            ALLOW_NO_BIND
        };

        Label(Assembler &assembler, const std::string &name, const LabelPolicy &policy = MUST_BIND);

        operator std::string() const { return _name; }
    };

    // this assembler does not have all mips command!
    class Assembler
    {
    private:
        static constexpr int _IDENTATION = 4;

        CodeBuffer &_code;
        int _ident;

        static constexpr int32_t _POP_OFFSET = 4;
        static constexpr int32_t _PUSH_OFFSET = -4; // stack grows down

        std::set<Register::REG> _used_registers;            // track registers
        std::unordered_map<std::string, bool> _used_labels; // is label with given was binded?

        // some reserved registers
        const Register _sp;
        const Register _ra;
        const Register _fp;
        const Register _zero;

        // current sp offset
        int _sp_offset;

        const static std::unordered_map<std::string, int> _special_symbols; // idk, spim requries some strings to be converted to int
        const static std::regex _special_symbols_regex;

        // .ascii str
        void ascii(const std::string &str);

        CODEGEN_FULL_VERBOSE_ONLY(Logger _logger;);

    public:
        Assembler(CodeBuffer &code);
        ~Assembler();

        inline const Register &sp() const { return _sp; }
        inline const Register &ra() const { return _ra; }
        inline const Register &fp() const { return _fp; }
        inline const Register &zero() const { return _zero; }

        // .data
        inline void data_section() { _code.save(std::string(_ident, ' ') + ".data"); }
        // .text
        inline void text_section() { _code.save(std::string(_ident, ' ') + ".text"); }

        // .align words
        inline void align(const int32_t &words) { _code.save(std::string(_ident, ' ') + ".align\t" + std::to_string(words)); }
        // .global symbol
        inline void global(const Label &symbol) { _code.save(std::string(_ident, ' ') + ".globl\t" + static_cast<std::string>(symbol)); }
        // .word value
        inline void word(const int32_t &value) { _code.save(std::string(_ident, ' ') + ".word\t" + std::to_string(value)); }
        // .word symbol (like pointer)
        inline void word(const Label &symbol) { _code.save(std::string(_ident, ' ') + ".word\t" + static_cast<std::string>(symbol)); }
        // .byte value
        inline void byte(const int8_t &value) { _code.save(std::string(_ident, ' ') + ".byte\t" + std::to_string(value)); }
        // emit string with escape sequencies
        void encode_string(const std::string &str);

        // store reg to memory location
        void sw(const Register &from_reg, const Register &to_reg, const int32_t &offset);
        // load word
        void lw(const Register &to_reg, const Register &from_reg, const int32_t &offset);
        // load address
        void la(const Register &to_reg, const Label &label);
        // move
        void move(const Register &result_reg, const Register &from_reg);
        // load immediate
        void li(const Register &reg, const int32_t &imm);

        // add immediate unsigned
        void addiu(const Register &result_reg, const Register &operand_reg, const int32_t &imm);

        // push reg value on stack
        void push(const Register &reg);
        // push value to reg from stack
        void pop(const Register &reg);
        void pop();

        // subtract
        void sub(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // add
        void add(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // addu
        void addu(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // multiply
        void mul(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // divide
        void div(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

        // xor
        void xor_(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // xor with immediate
        void xori(const Register &result_reg, const Register &op_reg, const int32_t &imm);
        // less than
        void slt(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);
        // shift left logical
        void sll(const Register &result_reg, const Register &op1_reg, const int32_t &imm);

        // branch on less than or equal
        void ble(const Register &op1_reg, const Register &op2_reg, const Label &label);
        // branch on not equal
        void bne(const Register &op1_reg, const Register &op2_reg, const Label &label);
        // branch on greater than
        void bgt(const Register &op1_reg, const Register &op2_reg, const Label &label);
        void bgt(const Register &op1_reg, const int32_t &op2_reg, const Label &label);
        // branch on less than
        void blt(const Register &op1_reg, const Register &op2_reg, const Label &label);
        void blt(const Register &op1_reg, const int32_t &op2_reg, const Label &label);
        // jump
        void j(const Label &label);
        // branch on equal
        void beq(const Register &op1_reg, const Register &op2_reg, const Label &label);
        void beq(const Register &op1_reg, const int32_t &op2_reg, const Label &label);
        // jump and link
        void jal(const Label &label);
        // jump and link register
        void jalr(const Register &reg);
        // jump register
        void jr(const Register &reg);

        // check if labels are binded at least in one assembler
        static void cross_resolve_labels(Assembler &asm1, Assembler &asm2);

        inline int sp_offset() const { return _sp_offset; }
        void set_sp_offset(const int32_t &offset) { _sp_offset = offset; }

#ifdef CODEGEN_FULL_VERBOSE
        void dump();
        inline void set_parent_logger(Logger *logger) { _logger.set_parent_logger(logger); }
#endif // CODEGEN_FULL_VERBOSE

        friend class AssemblerMarkSection;
        friend class Register;
        friend class Label;
    };

    class AssemblerMarkSection
    {
    private:
        Assembler &_asm;

    public:
        AssemblerMarkSection(Assembler &assembler, const Label &label);
    };
};