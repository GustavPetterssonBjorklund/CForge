#include "instruction_set.hpp"
#include <algorithm>
#include <stdexcept>

namespace cforge
{

    const std::unordered_map<std::string_view, InstructionInfo> InstructionSet::kInstructions = {
        // R‑type (OP, opcode = 0x33)
        {"add", InstructionInfo{0x33, 3, 4, false}},
        {"sub", InstructionInfo{0x33, 3, 4, false}},
        {"sll", InstructionInfo{0x33, 3, 4, false}},
        {"slt", InstructionInfo{0x33, 3, 4, false}},
        {"sltu", InstructionInfo{0x33, 3, 4, false}},
        {"xor", InstructionInfo{0x33, 3, 4, false}},
        {"srl", InstructionInfo{0x33, 3, 4, false}},
        {"sra", InstructionInfo{0x33, 3, 4, false}},
        {"or", InstructionInfo{0x33, 3, 4, false}},
        {"and", InstructionInfo{0x33, 3, 4, false}},

        // I‑type arithmetic (OP-IMM, opcode = 0x13)
        {"addi", InstructionInfo{0x13, 3, 4, false}},
        {"slti", InstructionInfo{0x13, 3, 4, false}},
        {"sltiu", InstructionInfo{0x13, 3, 4, false}},
        {"xori", InstructionInfo{0x13, 3, 4, false}},
        {"ori", InstructionInfo{0x13, 3, 4, false}},
        {"andi", InstructionInfo{0x13, 3, 4, false}},
        {"slli", InstructionInfo{0x13, 3, 4, false}},
        {"srli", InstructionInfo{0x13, 3, 4, false}},
        {"srai", InstructionInfo{0x13, 3, 4, false}},

        // Loads (LOAD, opcode = 0x03)
        {"lb", InstructionInfo{0x03, 3, 4, false}},
        {"lh", InstructionInfo{0x03, 3, 4, false}},
        {"lw", InstructionInfo{0x03, 3, 4, false}},
        {"lbu", InstructionInfo{0x03, 3, 4, false}},
        {"lhu", InstructionInfo{0x03, 3, 4, false}},

        // Stores (STORE, opcode = 0x23)
        {"sb", InstructionInfo{0x23, 3, 4, false}},
        {"sh", InstructionInfo{0x23, 3, 4, false}},
        {"sw", InstructionInfo{0x23, 3, 4, false}},

        // Branches (BRANCH, opcode = 0x63)
        {"beq", InstructionInfo{0x63, 3, 4, false}},
        {"bne", InstructionInfo{0x63, 3, 4, false}},
        {"blt", InstructionInfo{0x63, 3, 4, false}},
        {"bge", InstructionInfo{0x63, 3, 4, false}},
        {"bltu", InstructionInfo{0x63, 3, 4, false}},
        {"bgeu", InstructionInfo{0x63, 3, 4, false}},

        // Upper immediates
        {"lui", InstructionInfo{0x37, 2, 4, false}},   // U‑type
        {"auipc", InstructionInfo{0x17, 2, 4, false}}, // U‑type

        // Jumps
        {"jal", InstructionInfo{0x6F, 2, 4, false}},  // J‑type
        {"jalr", InstructionInfo{0x67, 3, 4, false}}, // I‑type

        // Pseudo-instructions (translated into real ones at assembly time)
        {"nop", InstructionInfo{0x13, 0, 4, false}},  // pseudo: addi x0, x0, 0
        {"mv", InstructionInfo{0x13, 2, 4, false}},   // pseudo: addi rd, rs, 0
        {"li", InstructionInfo{0x13, 2, 4, false}},   // pseudo: addi/ori/lui/etc.
        {"ret", InstructionInfo{0x67, 0, 4, false}},  // pseudo: jalr x0, x1, 0
        {"call", InstructionInfo{0x6F, 1, 4, false}}, // pseudo: auipc + jalr
        {"tail", InstructionInfo{0x6F, 1, 4, false}}, // pseudo: jalr x0, ...
        {"j", InstructionInfo{0x6F, 1, 4, false}},    // pseudo: jal x0, label
        {"jr", InstructionInfo{0x67, 1, 4, false}},   // pseudo: jalr x0, rs, 0
        {"seqz", InstructionInfo{0x13, 2, 4, false}}, // pseudo: sltiu rd, rs, 1
        {"snez", InstructionInfo{0x33, 2, 4, false}}, // pseudo: sltu rd, x0, rs
        {"sltz", InstructionInfo{0x33, 2, 4, false}}, // pseudo: slt rd, rs, x0
        {"sgtz", InstructionInfo{0x33, 2, 4, false}}, // pseudo: slt rd, x0, rs
    };

    const std::unordered_map<std::string_view, uint8_t> InstructionSet::kRegisters = {
        {"r0", 0x00}, {"r1", 0x01}, {"r2", 0x02}, {"r3", 0x03}, {"r4", 0x04}, {"r5", 0x05}, {"r6", 0x06}, {"r7", 0x07}, {"sp", 0x0E}, {"pc", 0x0F}};

    const std::unordered_set<std::string_view> InstructionSet::kDirectives = {
        ".section", ".globl", ".data", ".text", ".byte", ".word", ".dword", ".ascii", ".align", ".space"};

    const std::unordered_map<std::string_view, size_t> InstructionSet::kValidDataTypes = {
        {".byte", 1}, {".half", 2}, {".word", 4}, {".dword", 8}, {".ascii", 1}};

    const std::unordered_set<std::string_view> InstructionSet::kValidDataTypeSections = {
        ".data", ".bss", ".rodata"};

    // FIX: Implement all valid registers
    const std::unordered_map<std::string_view, uint8_t> InstructionSet::kRegisterMap = {
        {"r0", 0x00}, {"r1", 0x01}, {"r2", 0x02}, {"r3", 0x03}, {"r4", 0x04}, {"r5", 0x05}, {"r6", 0x06}, {"r7", 0x07}, {"sp", 0x0E}, {"pc", 0x0F}};

    bool InstructionSet::IsValidDirective(std::string_view directive)
    {
        return kDirectives.find(directive) != kDirectives.end();
    }

    bool InstructionSet::IsValidInstruction(std::string_view mnemonic)
    {
        return kInstructions.find(mnemonic) != kInstructions.end();
    }

    bool InstructionSet::IsValidDataType(std::string_view data_type)
    {
        return kValidDataTypes.find(data_type) != kValidDataTypes.end();
    }

    bool InstructionSet::IsValidDataTypeSection(std::string_view section)
    {
        return kValidDataTypeSections.find(section) != kValidDataTypeSections.end();
    }

    bool InstructionSet::IsValidRegister(std::string_view reg)
    {
        return kRegisters.find(reg) != kRegisters.end();
    }

    const InstructionInfo *InstructionSet::GetInstructionInfo(std::string_view mnemonic)
    {
        auto it = kInstructions.find(mnemonic);

        if (it == kInstructions.end())
        {
            throw Error("Unknown instruction: " + std::string(mnemonic));
        }
        return &it->second; // Return pointer to the InstructionInfo
    }

    std::vector<uint8_t> InstructionSet::GetDataBytes(
        const std::string_view data_type,
        const std::vector<std::string_view> &data)
    {
        auto it = kValidDataTypes.find(data_type);
        if (it == kValidDataTypes.end())
        {
            throw Error("Invalid data type: " + std::string(data_type));
        }

        // Convert the data to bytes based on the type
        size_t entry_size = it->second;
        std::vector<uint8_t> bytes;
        bytes.reserve(entry_size * data.size());

        for (const auto &value : data)
        {
            // Note that the value could be in hex, bin, or decimal format
            if (value.empty())
            {
                throw Error("Empty value for data type: " + std::string(data_type));
            }
            try
            {
                // Convert the value to an integer
                int64_t int_value = std::stoll(std::string(value), nullptr, 0);

                // Ensure the value fits in the specified entry size
                if (int_value < 0 || int_value >= (1ULL << (entry_size * 8)))
                {
                    throw Error("Value out of range for data type: " + std::string(data_type) + " - " + std::string(value));
                }

#ifdef LITTLE_ENDIAN
                // Handle different data types
                for (size_t i = 0; i < entry_size; ++i)
                {
                    bytes.push_back(static_cast<uint8_t>((int_value >> (i * 8)) & 0xFF));
                }
#else
                for (size_t i = 0; i < entry_size; ++i)
                {
                    bytes.push_back(static_cast<uint8_t>((int_value >> ((entry_size - 1 - i) * 8)) & 0xFF));
                }
#endif
            }
            catch (const std::invalid_argument &)
            {
                throw Error("Invalid value for data type: " + std::string(data_type) + " - " + std::string(value));
            }
        }

        bytes.shrink_to_fit(); // Ensure the vector is sized correctly
        return bytes;
    }

    size_t InstructionSet::CalculateDataSize(std::string_view data_type,
                                             const std::vector<std::string_view> &data)
    {
        auto it = kValidDataTypes.find(data_type);
        if (it == kValidDataTypes.end())
        {
            throw Error("Invalid data type: " + std::string(data_type));
        }

        size_t entry_size = it->second;
        return entry_size * data.size();
    }

    // FIX: This def does not handle instructions expanding to 8 bytes
    size_t InstructionSet::CalculateInstructionSize(std::string_view mnemonic,
                                                    const std::vector<std::string_view> &operands)
    {
        auto info = GetInstructionInfo(mnemonic);
        if (!info)
            return 0;

        size_t size = info->base_size;

        if (!info->variable_length)
        {
            return size;
        }

        // Check operands for size extensions
        for (const auto &operand : operands)
        {
            auto mode = DetermineAddressingMode(operand);

            switch (mode)
            {
            case AddressingMode::IMMEDIATE:
                // Immediate values require 4 extra bytes
                size += 4;
                break;

            case AddressingMode::MEMORY_DIRECT:
                // Direct memory addresses require 4 extra bytes
                size += 4;
                break;

            case AddressingMode::LABEL:
                // Labels will be resolved to addresses (4 bytes)
                size += 4;
                break;

            case AddressingMode::REGISTER:
            case AddressingMode::MEMORY_INDIRECT:
                // These are encoded in the base instruction
                break;
            }
        }

        return size;
    }

    AddressingMode InstructionSet::DetermineAddressingMode(std::string_view operand)
    {
        if (operand.empty())
            return AddressingMode::REGISTER;

        if (IsRegister(operand))
        {
            return AddressingMode::REGISTER;
        }

        if (IsImmediate(operand))
        {
            return AddressingMode::IMMEDIATE;
        }

        if (IsMemoryReference(operand))
        {
            // Check if it's indirect [r1] or direct [0x1000]
            if (operand.size() > 2 && operand.front() == '[' && operand.back() == ']')
            {
                std::string_view inner = operand.substr(1, operand.size() - 2);
                if (IsRegister(inner))
                {
                    return AddressingMode::MEMORY_INDIRECT;
                }
                else
                {
                    return AddressingMode::MEMORY_DIRECT;
                }
            }
        }

        // If it's not a register, immediate, or memory reference, assume it's a label
        return AddressingMode::LABEL;
    }

    bool InstructionSet::IsImmediate(std::string_view operand)
    {
        return !operand.empty() && operand.front() == '#';
    }

    bool InstructionSet::IsMemoryReference(std::string_view operand)
    {
        return operand.size() > 2 && operand.front() == '[' && operand.back() == ']';
    }

    bool InstructionSet::IsRegister(std::string_view operand)
    {
        return kRegisters.find(operand) != kRegisters.end();
    }
}