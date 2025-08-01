#include "instruction_set.hpp"

// std
#include <algorithm>
#include <stdexcept>
#include <iostream>

namespace cforge
{

    const std::unordered_map<std::string_view, InstructionInfo> InstructionSet::kInstructions = {
        // R‑type arithmetic & logic (opcode = R_TYPE - 0x33)
        {"add", {InstructionInfo::Type::R_TYPE, 0b000, 0b0000000, 3}},
        {"sub", {InstructionInfo::Type::R_TYPE, 0b000, 0b0100000, 3}},
        {"sll", {InstructionInfo::Type::R_TYPE, 0b001, 0b0000000, 3}},
        {"slt", {InstructionInfo::Type::R_TYPE, 0b010, 0b0000000, 3}},
        {"sltu", {InstructionInfo::Type::R_TYPE, 0b011, 0b0000000, 3}},
        {"xor", {InstructionInfo::Type::R_TYPE, 0b100, 0b0000000, 3}},
        {"srl", {InstructionInfo::Type::R_TYPE, 0b101, 0b0000000, 3}},
        {"sra", {InstructionInfo::Type::R_TYPE, 0b101, 0b0100000, 3}},
        {"or", {InstructionInfo::Type::R_TYPE, 0b110, 0b0000000, 3}},
        {"and", {InstructionInfo::Type::R_TYPE, 0b111, 0b0000000, 3}},

        // I‑type arithmetic / immediate (opcode = I_TYPE - 0x13)
        {"addi", {InstructionInfo::Type::I_TYPE, 0b000, /*func7 N/A*/ 0, 3}},
        {"slti", {InstructionInfo::Type::I_TYPE, 0b010, 0, 3}},
        {"sltiu", {InstructionInfo::Type::I_TYPE, 0b011, 0, 3}},
        {"xori", {InstructionInfo::Type::I_TYPE, 0b100, 0, 3}},
        {"ori", {InstructionInfo::Type::I_TYPE, 0b110, 0, 3}},
        {"andi", {InstructionInfo::Type::I_TYPE, 0b111, 0, 3}},
        {"slli", {InstructionInfo::Type::I_TYPE, 0b001, 0b0000000, 3}},
        {"srli", {InstructionInfo::Type::I_TYPE, 0b101, 0b0000000, 3}},
        {"srai", {InstructionInfo::Type::I_TYPE, 0b101, 0b0100000, 3}},

        // Loads (opcode = LOAD - 0x03)
        {"lb", {InstructionInfo::Type::LOAD, 0b000, 0, 2}},
        {"lh", {InstructionInfo::Type::LOAD, 0b001, 0, 2}},
        {"lw", {InstructionInfo::Type::LOAD, 0b010, 0, 2}},
        {"lbu", {InstructionInfo::Type::LOAD, 0b100, 0, 2}},
        {"lhu", {InstructionInfo::Type::LOAD, 0b101, 0, 2}},

        // Stores (opcode = STORE - 0x23)
        {"sb", {InstructionInfo::Type::STORE, 0b000, 0, 2}},
        {"sh", {InstructionInfo::Type::STORE, 0b001, 0, 2}},
        {"sw", {InstructionInfo::Type::STORE, 0b010, 0, 2}},

        // Branches (opcode = BRANCH - 0x63)
        {"beq", {InstructionInfo::Type::BRANCH, 0b000, 0, 3}},
        {"bne", {InstructionInfo::Type::BRANCH, 0b001, 0, 3}},
        {"blt", {InstructionInfo::Type::BRANCH, 0b100, 0, 3}},
        {"bge", {InstructionInfo::Type::BRANCH, 0b101, 0, 3}},
        {"bltu", {InstructionInfo::Type::BRANCH, 0b110, 0, 3}},
        {"bgeu", {InstructionInfo::Type::BRANCH, 0b111, 0, 3}},

        // Upper immediates (U‑type)
        {"lui", {InstructionInfo::Type::U_TYPE, 0, 0, 2}},
        {"auipc", {InstructionInfo::Type::U_TYPE, 0, 0, 2}},

        // Jumps
        {"jal", {InstructionInfo::Type::J_TYPE, 0, 0, 2}},
        {"jalr", {InstructionInfo::Type::J_TYPE, 0b000, 0, 2}}, // Note: This instruction is not actually a J_TYPE, but it's compiled in that group

        // Pseudo-instructions
        {"la", {InstructionInfo::Type::PSEUDO, 0, 0, 2}},
        {"j", {InstructionInfo::Type::PSEUDO, 0, 0, 2}}};

    const std::unordered_map<std::string_view, uint8_t> InstructionSet::kRegisters = {
        // Numeric names
        {"x0", 0x00},
        {"x1", 0x01},
        {"x2", 0x02},
        {"x3", 0x03},
        {"x4", 0x04},
        {"x5", 0x05},
        {"x6", 0x06},
        {"x7", 0x07},
        {"x8", 0x08},
        {"x9", 0x09},
        {"x10", 0x0A},
        {"x11", 0x0B},
        {"x12", 0x0C},
        {"x13", 0x0D},
        {"x14", 0x0E},
        {"x15", 0x0F},
        {"x16", 0x10},
        {"x17", 0x11},
        {"x18", 0x12},
        {"x19", 0x13},
        {"x20", 0x14},
        {"x21", 0x15},
        {"x22", 0x16},
        {"x23", 0x17},
        {"x24", 0x18},
        {"x25", 0x19},
        {"x26", 0x1A},
        {"x27", 0x1B},
        {"x28", 0x1C},
        {"x29", 0x1D},
        {"x30", 0x1E},
        {"x31", 0x1F},

        // ABI names
        {"zero", 0x00}, // Hard-wired zero
        {"ra", 0x01},   // Return address
        {"sp", 0x02},   // Stack pointer
        {"gp", 0x03},   // Global pointer
        {"tp", 0x04},   // Thread pointer
        {"t0", 0x05},
        {"t1", 0x06},
        {"t2", 0x07}, // Temporaries
        {"s0", 0x08},
        {"fp", 0x08}, // Saved register / frame pointer
        {"s1", 0x09}, // Saved register
        {"a0", 0x0A},
        {"a1", 0x0B}, // Function arguments / return values
        {"a2", 0x0C},
        {"a3", 0x0D},
        {"a4", 0x0E},
        {"a5", 0x0F}, // Function arguments
        {"a6", 0x10},
        {"a7", 0x11}, // Function arguments
        {"s2", 0x12},
        {"s3", 0x13},
        {"s4", 0x14},
        {"s5", 0x15}, // Saved registers
        {"s6", 0x16},
        {"s7", 0x17},
        {"s8", 0x18},
        {"s9", 0x19}, // Saved registers
        {"s10", 0x1A},
        {"s11", 0x1B}, // Saved registers
        {"t3", 0x1C},
        {"t4", 0x1D},
        {"t5", 0x1E},
        {"t6", 0x1F}, // Temporaries
    };

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
            throw Error("Instruction info not found for: " + std::string(mnemonic));
        }
        return &it->second; // Return pointer to the InstructionInfo
    }

    uint8_t InstructionSet::GetRegisterCode(std::string_view reg)
    {
        auto it = kRegisters.find(reg);
        std::cout << "Looking for register: \"" << reg << "\"\n";
        if (it == kRegisters.end())
        {
            throw std::runtime_error("Invalid register: " + std::string(reg));
        }
        return it->second;
    }

    std::vector<uint8_t> InstructionSet::GetDataBytes(
        const std::string data_type,
        const std::vector<std::string> &data)
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

                // Handle different data types
                for (size_t i = 0; i < entry_size; ++i)
                {
                    bytes.push_back(static_cast<uint8_t>((int_value >> (i * 8)) & 0xFF));
                }
            }
            catch (const std::invalid_argument &)
            {
                throw Error("Invalid value for data type: " + std::string(data_type) + " - " + std::string(value));
            }
        }

        bytes.shrink_to_fit(); // Ensure the vector is sized correctly
        return bytes;
    }

    CompiledInstruction InstructionSet::CompileInstruction(
        std::string mnemonic,
        const std::vector<std::string> &operands,
        uint32_t line)
    {
        const InstructionInfo *info = GetInstructionInfo(mnemonic);

        /**
         * @brief Unique instruction ID for relocations.
         * @details This is a really sketchy way to handle instruction IDs, by appending 1 for each
         * 4-byte instruction compiled, which (should) make Linking possible.
         */
        static size_t instruction_id = 0;
        std::cout << "instruction id: " << instruction_id << "\n";
        try
        {
            switch (info->opcode)
            {
            case InstructionInfo::Type::R_TYPE:
                ++instruction_id;
                return CompileRTypeInstruction(info, operands);
            case InstructionInfo::Type::I_TYPE:
                ++instruction_id;
                return CompileITypeInstruction(info, operands);
            // case InstructionInfo::Type::LOAD:
            //     ++instruction_id;
            //     return CompileLoadStoreInstruction(info, operands);
            // case InstructionInfo::Type::STORE:
            //     ++instruction_id;
            //     return CompileLoadStoreInstruction(info, operands);
            case InstructionInfo::Type::J_TYPE:
                ++instruction_id;
                return CompileJTypeInstruction(instruction_id, mnemonic, info, operands);
            case InstructionInfo::Type::PSEUDO:
                // `instruction_id` incremented by CompilePseudoInstruction
                return CompilePseudoInstruction(instruction_id, mnemonic, info, operands);
            default:
                return CompiledInstruction{};
            }
        }
        catch (const std::exception &e)
        {
            throw Error(mnemonic, line, e.what());
        }
    }

    CompiledInstruction InstructionSet::CompileRTypeInstruction(
        const InstructionInfo *info,
        const std::vector<std::string> &operands)
    {
        // Expect exactly 3 operands for R-type instructions
        if (operands.size() != 3)
        {
            throw std::runtime_error("R-type instruction requires exactly 3 operands: ");
        }

        // Convert register operands to codes
        uint8_t rd = GetRegisterCode(operands[0]);
        uint8_t rs1 = GetRegisterCode(operands[1]);
        uint8_t rs2 = GetRegisterCode(operands[2]);

        // Create the instruction bytes
        CompiledInstruction instruction;
        instruction.bytes.resize(4);

        uint32_t inst =
            (static_cast<uint32_t>(info->func7) << 25) |
            (static_cast<uint32_t>(rs2) << 20) |
            (static_cast<uint32_t>(rs1) << 15) |
            (static_cast<uint32_t>(info->func3) << 12) |
            (static_cast<uint32_t>(rd) << 7) |
            (static_cast<uint32_t>(info->opcode));

        instruction.bytes[0] = inst & 0xFF;
        instruction.bytes[1] = (inst >> 8) & 0xFF;
        instruction.bytes[2] = (inst >> 16) & 0xFF;
        instruction.bytes[3] = (inst >> 24) & 0xFF;

        return instruction;
    }

    CompiledInstruction InstructionSet::CompileITypeInstruction(
        const InstructionInfo *info,
        const std::vector<std::string> &operands)
    {
        // Expect 2 register operands and 1 immediate value
        if (operands.size() != 3)
        {
            throw std::runtime_error("I-type instruction requires exactly 3 operands: ");
        }

        // Convert register operands to codes
        uint8_t rd = GetRegisterCode(operands[0]);
        uint8_t rs1 = GetRegisterCode(operands[1]);
        // Parse the immediate value
        int32_t imm = 0;
        try
        {
            imm = std::stoi(std::string(operands[2]), nullptr, 0);
        }
        catch (const std::invalid_argument &)
        {
            throw Error("Invalid immediate value: " + std::string(operands[2]));
        }

        // Create the instruction bytes
        CompiledInstruction instruction;
        instruction.bytes.resize(4);
        uint32_t inst =
            (static_cast<uint32_t>((imm | 0) & 0xFFF) << 20) | // Immediate
            (static_cast<uint32_t>(rs1) << 15) |               // rs1
            (static_cast<uint32_t>(info->func3) << 12) |       // func3
            (static_cast<uint32_t>(rd) << 7) |                 // rd
            (static_cast<uint32_t>(info->opcode));             // opcode
        instruction.bytes[0] = inst & 0xFF;
        instruction.bytes[1] = (inst >> 8) & 0xFF;
        instruction.bytes[2] = (inst >> 16) & 0xFF;
        instruction.bytes[3] = (inst >> 24) & 0xFF;

        return instruction;
    }

    // FIX: Doesn't work for jalr
    CompiledInstruction InstructionSet::CompileJTypeInstruction(
        const size_t &instruction_id,
        const std::string mnemonic,
        const InstructionInfo *info,
        const std::vector<std::string> &operands)
    {
        if (mnemonic == "jal")
        {
            // Expect 2 operands: rd and label
            if (operands.size() != 2)
            {
                throw std::runtime_error("J-type instruction requires exactly 2 operands: ");
            }

            // Convert register operand to code
            uint8_t rd = GetRegisterCode(operands[0]);
            std::string label = std::string(operands[1]);

            // Create the instruction bytes
            CompiledInstruction instruction;
            instruction.bytes.resize(4);
            uint32_t inst =
                (static_cast<uint32_t>(rd) << 7) |     // rd
                (static_cast<uint32_t>(info->opcode)); // opcode

            instruction.bytes[0] = inst & 0xFF;
            instruction.bytes[1] = (inst >> 8) & 0xFF;
            instruction.bytes[2] = (inst >> 16) & 0xFF;
            instruction.bytes[3] = (inst >> 24) & 0xFF;

            // Add relocation entry for the label
            instruction.relocations.push_back({
                RelocationEntry::Type::R_RISC_V_JAL,
                ".text", // Section must be text TODO: make sure this is checked before this point
                instruction_id,
                label,
            });

            return instruction;
        }
        else if (mnemonic == "jalr")
        {
            throw std::runtime_error("jalr is not implemented yet");
        }
        else
        {
            throw std::runtime_error("Invalid J-type mnemonic: " + std::string(mnemonic));
        }
    }

    CompiledInstruction InstructionSet::CompilePseudoInstruction(
        size_t &instruction_id,
        const std::string mnemonic,
        const InstructionInfo *info,
        const std::vector<std::string> &operands)
    {
        CompiledInstruction instruction;
        // TODO: Don't assume all pseudo-instructions are 4 bytes

        if (mnemonic == "la")
        {
            // Load address pseudo-instruction
            if (operands.size() != 2)
            {
                throw std::runtime_error("la pseudo-instruction requires exactly 2 operands: ");
            }

            std::string reg = operands[0];

            uint8_t rd = GetRegisterCode(reg);

            // Get the address (label) to load
            std::string label = std::string(operands[1]);

            // Compile as addi replacing label with 0
            instruction = CompileITypeInstruction(GetInstructionInfo("addi"), {reg, "zero", "0"});

            // Add a relocation entry for the label
            instruction.relocations.push_back({
                RelocationEntry::Type::R_RISC_V_LO12_I,
                ".text",        // Section must be text TODO: make sure this is checked before this point
                instruction_id, // Instruction ID
                label,
            });

            // Increment the instruction ID
            ++instruction_id;
        }
        else if (mnemonic == "j")
        {
            instruction = CompileJTypeInstruction(const_cast<size_t &>(instruction_id),
                                                  "jal", GetInstructionInfo("jal"),
                                                  std::vector<std::string>{"x0",
                                                                           operands[0]});
            ++instruction_id; // Don't forget to increment the instruction ID for relocations
        }
        else
        {
            throw std::runtime_error("Unsupported pseudo-instruction");
        }

        return instruction;
    }

    size_t InstructionSet::CalculateDataSize(std::string_view data_type,
                                             const std::vector<std::string> &data)
    {
        auto it = kValidDataTypes.find(data_type);
        if (it == kValidDataTypes.end())
        {
            throw Error("Invalid data type: " + std::string(data_type));
        }

        size_t entry_size = it->second;
        return entry_size * data.size();
    }

    size_t InstructionSet::CalculateInstructionSize(std::string mnemonic,
                                                    const std::vector<std::string> &operands)
    {
        return 4;
    }

}