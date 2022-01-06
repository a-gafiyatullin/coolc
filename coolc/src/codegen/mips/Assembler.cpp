#include "codegen/mips/Assembler.h"

using namespace codegen;

CodeBuffer &CodeBuffer::operator+=(const CodeBuffer &buffer)
{
    _buffer += buffer._buffer;
    return *this;
}

// ----------------------------------------------- Regsiters -----------------------------------------------
const std::vector<std::string> Register::REG_TO_STR = {"$sp", "$fp", "$ra", "$t0", "$t1", "$t2", "$t3",
                                                       "$t4", "$t5", "$t6", "$a0", "$a1", "$s0", "$zero"};

Register::Register(Assembler &assembler, const Reg &reg) : _asm(assembler), _reg(reg)
{
    // guarantee that we have not use this register yet
    if (_asm._used_registers.find(_reg) != _asm._used_registers.end())
    {
        CODEGEN_VERBOSE_ONLY(_asm._logger->log("Register " + get_reg_name(reg) + " is being used!"));
        assert(false && "Register::Register: register was already allocated!");
    }
    _asm._used_registers.insert(_reg);
}

Register::~Register()
{
    const auto reg_ptr = _asm._used_registers.find(_reg);
    assert(reg_ptr != _asm._used_registers.end()); // imposible
    _asm._used_registers.erase(reg_ptr);
}

// ----------------------------------------------- Labels -----------------------------------------------
Label::Label(Assembler &assembler, const std::string &name, const LabelPolicy &policy) : _name(name)
{
    CODEGEN_VERBOSE_ONLY(assembler._logger->log("Create Label: " + name));
    assembler._used_labels.insert(std::make_pair(name, policy));
}
// ----------------------------------------------- Assembler -----------------------------------------------
const std::unordered_map<std::string, int> Assembler::SPECIAL_SYMBOLS = {{"\\", 92}};
const std::regex Assembler::SPECIAL_SYMBOLS_REGEX("\\\\");

Assembler::Assembler(CodeBuffer &code)
    : _code(code), _ident(IDENTATION), _sp(*this, Register::$sp), _ra(*this, Register::$ra), _fp(*this, Register::$fp),
      _sp_offset(0), _zero(*this, Register::$zero)
{
}

void Assembler::sw(const Register &from_reg, const Register &to_reg, const int32_t &offset)
{
    _code.save(std::string(_ident, ' ') + "sw\t\t" + static_cast<std::string>(from_reg) + " " + std::to_string(offset) +
               "(" + static_cast<std::string>(to_reg) + ")");
}

void Assembler::addiu(const Register &result_reg, const Register &operand_reg, const int32_t &imm)
{
    _code.save(std::string(_ident, ' ') + "addiu\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(operand_reg) + " " + std::to_string(imm));
}

void Assembler::push(const Register &reg)
{
    sw(reg, _sp, 0);
    addiu(_sp, _sp, PUSH_OFFSET);
    _sp_offset += PUSH_OFFSET;
}

void Assembler::lw(const Register &to_reg, const Register &from_reg, const int32_t &offset)
{
    _code.save(std::string(_ident, ' ') + "lw\t\t" + static_cast<std::string>(to_reg) + " " + std::to_string(offset) +
               "(" + static_cast<std::string>(from_reg) + ")");
}

void Assembler::la(const Register &to_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "la\t\t" + static_cast<std::string>(to_reg) + " " +
               static_cast<std::string>(label));
}

void Assembler::move(const Register &result_reg, const Register &from_reg)
{
    _code.save(std::string(_ident, ' ') + "move\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(from_reg));
}

void Assembler::sll(const Register &result_reg, const Register &op_reg, const int32_t &imm)
{
    _code.save(std::string(_ident, ' ') + "sll\t\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op_reg) + " " + std::to_string(imm));
}

void Assembler::pop(const Register &reg)
{
    lw(reg, _sp, POP_OFFSET);
    pop();
}

void Assembler::pop()
{
    addiu(_sp, _sp, POP_OFFSET);
    _sp_offset += POP_OFFSET;
}

void Assembler::sub(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "sub\t\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::add(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "add\t\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::addu(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "addu\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::mul(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "mul\t\t" + " " + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::div(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "div\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg));
    _code.save(std::string(_ident, ' ') + "mflo\t" + static_cast<std::string>(result_reg));
}

void Assembler::slt(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "slt\t\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::ble(const Register &op1_reg, const Register &op2_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "ble\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg) + " " + static_cast<std::string>(label));
}

void Assembler::beq(const Register &op1_reg, const Register &op2_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "beq\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg) + " " + static_cast<std::string>(label));
}

void Assembler::beq(const Register &op1_reg, const int32_t &op2_imm, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "beq\t\t" + static_cast<std::string>(op1_reg) + " " +
               std::to_string(op2_imm) + " " + static_cast<std::string>(label));
}

void Assembler::j(const Label &label)
{
    _code.save(std::string(_ident, ' ') + "j\t\t" + static_cast<std::string>(label));
}

void Assembler::li(const Register &reg, const int32_t &imm)
{
    _code.save(std::string(_ident, ' ') + "li\t\t" + static_cast<std::string>(reg) + " " + std::to_string(imm));
}

void Assembler::jal(const Label &label)
{
    _code.save(std::string(_ident, ' ') + "jal\t\t" + static_cast<std::string>(label));
}

void Assembler::xorr(const Register &result_reg, const Register &op1_reg, const Register &op2_reg)
{
    _code.save(std::string(_ident, ' ') + "xor\t\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op1_reg) + " " + static_cast<std::string>(op2_reg));
}

void Assembler::xori(const Register &result_reg, const Register &op_reg, const int32_t &imm)
{
    _code.save(std::string(_ident, ' ') + "xori\t" + static_cast<std::string>(result_reg) + " " +
               static_cast<std::string>(op_reg) + " " + std::to_string(imm));
}

Assembler::~Assembler()
{
    std::for_each(_used_labels.begin(), _used_labels.end(), [&](const auto &label) {
        if (!label.second)
        {
            CODEGEN_VERBOSE_ONLY(_logger->log("Label " + label.first + " was not binded!"));
            assert(false && "Assembler::~Assembler: non-binded label found!");
        }
    });
}

void Assembler::ascii(const std::string &str)
{
    _code.save(std::string(_ident, ' ') + ".ascii\t\"" +
               std::regex_replace(std::regex_replace(str, std::regex("\n"), "\\n"), std::regex("\""), "\\\"") + "\"");
}

void Assembler::encode_string(const std::string &str)
{
    if (str.empty())
    {
        return;
    }

    auto match_begin = std::sregex_iterator(str.begin(), str.end(), SPECIAL_SYMBOLS_REGEX);
    auto match_end = std::sregex_iterator();

    // it is normal string
    if (match_begin == match_end)
    {
        ascii(str);
        return;
    }

    // string with special symbols
    for (auto it = match_begin; it != match_end;)
    {
        const auto this_match = it;
        const auto next_match = ++it;

        byte(SPECIAL_SYMBOLS.at(this_match->str())); // encode special symbols in bytes

        const int start_pos = this_match->position() + this_match->str().length();
        // the rest of the string up to the next special symbol encode in ascii
        if (next_match != match_end && next_match->position() != this_match->position() + this_match->str().length())
        {
            ascii(str.substr(start_pos, next_match->position() - start_pos));
        }
        else if (next_match == match_end)
        {
            if (start_pos < str.length())
            {
                ascii(str.substr(start_pos));
            }
            break;
        }
    }
}

void Assembler::jr(const Register &reg)
{
    _code.save(std::string(_ident, ' ') + "jr\t\t" + static_cast<std::string>(reg));
}

void Assembler::jalr(const Register &reg)
{
    _code.save(std::string(_ident, ' ') + "jalr\t" + static_cast<std::string>(reg));
}

void Assembler::cross_resolve_labels(Assembler &asm1, Assembler &asm2)
{
    auto adjust_bind_state = [](Assembler &other_asm, std::pair<const std::string, bool> &label) {
        if (!label.second)
        {
            const auto this_label = other_asm._used_labels.find(label.first);
            if (this_label != other_asm._used_labels.end() && this_label->second)
            {
                label.second = true;
            }
        }
    };

#ifdef DEBUG
    asm1._logger->log("ASSEMBLERS BEFORE CROSS-RESOLVING:\nASM1:");
    asm1.dump();
    asm2._logger->log("ASM2");
    asm2.dump();
#endif // DEBUG

    std::for_each(asm1._used_labels.begin(), asm1._used_labels.end(),
                  [&asm2, &adjust_bind_state](auto &label) { adjust_bind_state(asm2, label); });
    std::for_each(asm2._used_labels.begin(), asm2._used_labels.end(),
                  [&asm1, &adjust_bind_state](auto &label) { adjust_bind_state(asm1, label); });

#ifdef DEBUG
    asm1._logger->log("ASSEMBLERS AFTER CROSS-RESOLVING:\nASM1:");
    asm1.dump();
    asm2._logger->log("ASM2");
    asm2.dump();
#endif // DEBUG
}

void Assembler::bne(const Register &op1_reg, const Register &op2_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "bne\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg) + " " + static_cast<std::string>(label));
}

void Assembler::bgt(const Register &op1_reg, const Register &op2_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "bgt\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg) + " " + static_cast<std::string>(label));
}

void Assembler::bgt(const Register &op1_reg, const int32_t &op2_imm, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "bgt\t\t" + static_cast<std::string>(op1_reg) + " " +
               std::to_string(op2_imm) + " " + static_cast<std::string>(label));
}

void Assembler::blt(const Register &op1_reg, const Register &op2_reg, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "blt\t\t" + static_cast<std::string>(op1_reg) + " " +
               static_cast<std::string>(op2_reg) + " " + static_cast<std::string>(label));
}

void Assembler::blt(const Register &op1_reg, const int32_t &op2_imm, const Label &label)
{
    _code.save(std::string(_ident, ' ') + "blt\t\t" + static_cast<std::string>(op1_reg) + " " +
               std::to_string(op2_imm) + " " + static_cast<std::string>(label));
}

#ifdef DEBUG
void Assembler::dump()
{
    _logger->log("----------- Labels -----------");
    std::for_each(_used_labels.begin(), _used_labels.end(),
                  [&](const auto &label) { _logger->log(label.first + "\tstate: " + std::to_string(label.second)); });

    _logger->log("----------- Registers -----------");
    std::for_each(_used_registers.begin(), _used_registers.end(),
                  [&](const auto &reg) { _logger->log(Register::get_reg_name(reg)); });
}
#endif // DEBUG

// ----------------------------------------------- AssemblerMarkSection -----------------------------------------------
AssemblerMarkSection::AssemblerMarkSection(Assembler &assembler, const Label &label) : _asm(assembler)
{
    const std::string label_name = static_cast<std::string>(label);
    _asm._code.save(label_name + ":");

    CODEGEN_VERBOSE_ONLY(assert(_asm._used_labels.find(label_name) != _asm._used_labels.end())); // impossible

    // label was binded
    auto label_ptr = _asm._used_labels.find(label_name);
    if (label_ptr->second)
    {
        CODEGEN_VERBOSE_ONLY(assembler._logger->log("Label " + label_name + " been already binded!"));
        assert(false && "AssemblerMarkSection::AssemblerMarkSection: label has been already binded!");
    }
    label_ptr->second = true;
}