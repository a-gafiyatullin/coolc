#pragma once

#include "utils/Utils.h"
#include <algorithm>
#include <functional>
#include <regex>
#include <set>
#include <unordered_map>

namespace codegen
{

/**
 * @brief CodeBuffer contains the final assembler
 *
 */
class CodeBuffer
{
  private:
    std::string _buffer;

  public:
    CodeBuffer() = default;

    /**
     * @brief Construct a new Code Buffer
     *
     * @param buffer Initial buffer
     */
    explicit CodeBuffer(const CodeBuffer &buffer) : _buffer(buffer._buffer) {}

    /**
     * @brief Add new commands to buffer
     *
     * @param command New commands
     */
    inline void save(const std::string &command) { _buffer += command + "\n"; }

    /**
     * @brief Concat buffers
     *
     * @param buffer Other buffer
     * @return This buffer with other buffer's content
     */
    CodeBuffer &operator+=(const CodeBuffer &buffer);

    /**
     * @brief Cast to string
     *
     * @return Conten of the buffer
     */
    operator std::string() const { return _buffer; }
};

class Assembler;

/**
 * @brief Register represenantion in Assembler
 * Not all mips regiters!
 */
class Register
{
  public:
    /**
     * @brief Available registers
     *
     */
    enum Reg
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

    /**
     * @brief Reserve a register in Assembler
     *
     * @param reg Register name
     */
    explicit Register(const Reg &reg);

    /**
     * @brief Untrack Register from Assembler
     *
     */
    ~Register();

    /**
     * @brief Cast to string
     *
     * @return Register name
     */
    operator std::string() const { return REG_TO_STR[_reg]; }

    /**
     * @brief Get the register name as string
     *
     * @param reg Register name as Reg
     * @return Register name as string
     */
    inline static std::string reg_name(const Register::Reg &reg) { return REG_TO_STR[reg]; }

  private:
    static const std::vector<std::string> REG_TO_STR;

    const Reg _reg;
};

class AssemblerMarkSection;

/**
 * @brief Label is target for jumps
 *
 */
class Label
{
  private:
    const std::string _name;

  public:
    /**
     * @brief Controls if this label has to be binded or not
     *
     */
    enum LabelPolicy
    {
        MUST_BIND,
        ALLOW_NO_BIND
    };

    /**
     * @brief Construct a new Label
     *
     * @param name Label name
     * @param policy This label has to be binded or not
     */
    explicit Label(const std::string &name, const LabelPolicy &policy = MUST_BIND);

    /**
     * @brief Cast to string
     *
     * @return Label name
     */
    operator std::string() const { return _name; }
};

/**
 * @brief MIPS Assember
 * Not all MIPS commands!
 */
class Assembler
{
  private:
    static constexpr int IDENTATION = 4;

    CodeBuffer &_code;
    int _ident;

    static constexpr int32_t POP_OFFSET = 4;
    static constexpr int32_t PUSH_OFFSET = -4; // stack grows down

    static std::set<Register::Reg> UsedRegisters;            // track registers
    static std::unordered_map<std::string, bool> UsedLabels; // is label with given was binded?

    // some reserved registers
    static const Register SP;
    static const Register RA;
    static const Register FP;
    static const Register ZERO;

    // current sp offset
    int _sp_offset;

    const static std::unordered_map<std::string, int>
        SPECIAL_SYMBOLS; // idk, spim requries some strings to be converted to int
    const static std::regex SPECIAL_SYMBOLS_REGEX;

    // .ascii str
    void ascii(const std::string &str);

  public:
    /**
     * @brief Construct a new Assembler
     *
     * @param code Emit code to this buffer
     */
    explicit Assembler(CodeBuffer &code);

    /**
     * @brief sp register
     *
     * @return sp register
     */
    inline const Register &sp() const { return SP; }

    /**
     * @brief ra register
     *
     * @return ra register
     */
    inline const Register &ra() const { return RA; }

    /**
     * @brief fp register
     *
     * @return fp register
     */
    inline const Register &fp() const { return FP; }

    /**
     * @brief zero register
     *
     * @return zero register
     */
    inline const Register &zero() const { return ZERO; }

    /**
     * @brief Create data section
     *
     */
    inline void data_section() { _code.save(std::string(_ident, ' ') + ".data"); }

    /**
     * @brief Create text section
     *
     */
    inline void text_section() { _code.save(std::string(_ident, ' ') + ".text"); }

    /**
     * @brief Align
     *
     * @param words Words count
     */
    inline void align(const int32_t &words)
    {
        _code.save(std::string(_ident, ' ') + ".align\t" + std::to_string(words));
    }

    /**
     * @brief Declare global symbol
     *
     * @param symbol Label object
     */
    inline void global(const Label &symbol)
    {
        _code.save(std::string(_ident, ' ') + ".globl\t" + static_cast<std::string>(symbol));
    }

    /**
     * @brief Declare word
     *
     * @param value Word value
     */
    inline void word(const int32_t &value) { _code.save(std::string(_ident, ' ') + ".word\t" + std::to_string(value)); }

    /**
     * @brief Declare pointer
     *
     * @param symbol Label object
     */
    inline void word(const Label &symbol)
    {
        _code.save(std::string(_ident, ' ') + ".word\t" + static_cast<std::string>(symbol));
    }

    /**
     * @brief Declare byte
     *
     * @param value Byte value
     */
    inline void byte(const int8_t &value) { _code.save(std::string(_ident, ' ') + ".byte\t" + std::to_string(value)); }

    /**
     * @brief Emit string with escape sequencies
     *
     * @param str String
     */
    void encode_string(const std::string &str);

    /**
     * @brief Store register value to memory location
     *
     * @param from_reg Register
     * @param to_reg Base
     * @param offset Offset
     */
    void sw(const Register &from_reg, const Register &to_reg, const int32_t &offset);

    /**
     * @brief Load word
     *
     * @param to_reg Destination register
     * @param from_reg Base
     * @param offset Offset
     */
    void lw(const Register &to_reg, const Register &from_reg, const int32_t &offset);

    /**
     * @brief Load address
     *
     * @param to_reg Destination register
     * @param label Label object
     */
    void la(const Register &to_reg, const Label &label);

    /**
     * @brief Move
     *
     * @param result_reg Destination register
     * @param from_reg Source register
     */
    void move(const Register &result_reg, const Register &from_reg);

    /**
     * @brief Load immediate
     *
     * @param reg Destination register
     * @param imm Value
     */
    void li(const Register &reg, const int32_t &imm);

    /**
     * @brief Add immediate unsigned
     *
     * @param result_reg Destination register
     * @param operand_reg Operand register
     * @param imm Operand value
     */
    void addiu(const Register &result_reg, const Register &operand_reg, const int32_t &imm);

    /**
     * @brief Push register value on stack
     *
     * @param reg Register for push
     */
    void push(const Register &reg);

    /**
     * @brief Pop value to register from stack
     *
     * @param reg Register fot pop
     */
    void pop(const Register &reg);

    /**
     * @brief  Pop value from stack
     *
     */
    void pop();

    /**
     * @brief Subtract
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void sub(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Add
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void add(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Add unsigned
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void addu(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Multiply
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void mul(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Divide
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void div(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Xor
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void xorr(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Xor
     *
     * @param result_reg Destination register
     * @param op_reg Operand register
     * @param imm Operand value
     */
    void xori(const Register &result_reg, const Register &op_reg, const int32_t &imm);

    /**
     * @brief Check if first operand is lesser than second
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     */
    void slt(const Register &result_reg, const Register &op1_reg, const Register &op2_reg);

    /**
     * @brief Shift left logical
     *
     * @param result_reg Destination register
     * @param op1_reg Operand register
     * @param imm Number of bits
     */
    void sll(const Register &result_reg, const Register &op1_reg, const int32_t &imm);

    /**
     * @brief Branch on less than or equal
     *
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     * @param label Branch target
     */
    void ble(const Register &op1_reg, const Register &op2_reg, const Label &label);

    /**
     * @brief Branch on not equal
     *
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     * @param label Branch target
     */
    void bne(const Register &op1_reg, const Register &op2_reg, const Label &label);

    /**
     * @brief Branch on greater than
     *
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     * @param label Branch target
     */
    void bgt(const Register &op1_reg, const Register &op2_reg, const Label &label);

    /**
     * @brief Branch on greater than
     *
     * @param op1_reg Operand register
     * @param op2_imm Operand value
     * @param label Branch target
     */
    void bgt(const Register &op1_reg, const int32_t &op2_imm, const Label &label);

    /**
     * @brief Branch on less than
     *
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     * @param label Branch target
     */
    void blt(const Register &op1_reg, const Register &op2_reg, const Label &label);

    /**
     * @brief Branch on less than
     *
     * @param op1_reg Operand register
     * @param op2_imm Operand value
     * @param label Branch target
     */
    void blt(const Register &op1_reg, const int32_t &op2_imm, const Label &label);

    /**
     * @brief Uncinditional jump
     *
     * @param label Jump target
     */
    void j(const Label &label);

    /**
     * @brief Branch on equal
     *
     * @param op1_reg Operand register
     * @param op2_reg Operand register
     * @param label Branch target
     */
    void beq(const Register &op1_reg, const Register &op2_reg, const Label &label);

    /**
     * @brief Branch on equal
     *
     * @param op1_reg Operand register
     * @param op2_imm Operand value
     * @param label Branch target
     */
    void beq(const Register &op1_reg, const int32_t &op2_imm, const Label &label);

    /**
     * @brief Jump and link
     *
     * @param label Jump target
     */
    void jal(const Label &label);

    /**
     * @brief Jump and link by register value
     *
     * @param reg Address register
     */
    void jalr(const Register &reg);

    /**
     * @brief Jump by register value
     *
     * @param reg Address register
     */
    void jr(const Register &reg);

    /**
     * @brief Check if labels are binded at least in one assembler
     *
     */
    static void check_labels();

    /**
     * @brief Get current sp offset
     *
     * @return sp offset
     */
    inline int sp_offset() const { return _sp_offset; }

    /**
     * @brief Set sp offset
     *
     * @param offset New offset
     */
    void set_sp_offset(const int32_t &offset) { _sp_offset = offset; }

#ifdef DEBUG
    /**
     * @brief Dump registers and labels for this assembler
     *
     */
    static void dump();
#endif // DEBUG

    friend class AssemblerMarkSection;
    friend class Register;
    friend class Label;
};

/**
 * @brief AssemblerMarkSection binds Labels
 *
 */
class AssemblerMarkSection
{
  public:
    /**
     * @brief Construct a new AssemblerMarkSection and bind Label
     *
     * @param assembler Assembler that will control this label
     * @param label Label object
     */
    AssemblerMarkSection(Assembler &assembler, const Label &label);
};

}; // namespace codegen