#pragma once
#include "error.hpp"

// std
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <cstdint>
#include <vector>
#include <string>

namespace cforge
{

    enum class AddressingMode : uint8_t
    {
        REGISTER,           // mov r1, r2
        IMMEDIATE,          // mov r1, #100
        REGISTER_IMMEDIATE, // mov r1, r2, #100
        MEMORY_DIRECT,      // mov r1, [0x1000]
        MEMORY_INDIRECT,    // mov r1, [r2]
        LABEL               // jmp label_name
    };

    struct InstructionInfo
    {
        uint8_t opcode;
        uint8_t operand_count;
        uint8_t base_size;    // Base instruction size in bytes
        bool variable_length; // Can this instruction have variable length?
    };

    class InstructionSet
    {
    public:
        // Core instruction information
        static const std::unordered_map<std::string_view, InstructionInfo> kInstructions;

        // Register mappings
        static const std::unordered_map<std::string_view, uint8_t> kRegisters;

        // Valid directives
        static const std::unordered_set<std::string_view> kDirectives;

        // Valid data types / map of data type entry size (in bytes)
        static const std::unordered_map<std::string_view, size_t> kValidDataTypes;

        // Sections where data types may be used
        static const std::unordered_set<std::string_view> kValidDataTypeSections;

        // Valid register definitions / map for converting to opcodes
        static const std::unordered_map<std::string_view, uint8_t> kRegisterMap;

        // Validation methods
        static bool IsValidInstruction(std::string_view mnemonic);
        static bool IsValidDirective(std::string_view directive);
        static bool IsValidDataType(std::string_view data_type);
        static bool IsValidDataTypeSection(std::string_view section);
        static bool IsValidRegister(std::string_view reg);

        static const InstructionInfo *GetInstructionInfo(std::string_view mnemonic);
        static uint8_t GetRegisterCode(std::string_view reg);

        /**
         * Get the bytes for a data type and its values.
         * @param data_type Type of data, must be one of the valid data types.
         * @param data Values to convert to bytes.
         * @return Vector of bytes representing the data.
         */
        static std::vector<uint8_t> GetDataBytes(
            const std::string_view data_type,
            const std::vector<std::string_view> &data);

        /**
         * Calculate size of data based on type and amount of values
         * @attention This method will return UB if data type is invalid
         * @param data_type Type of data MUST be one of the valid data types
         * as defined in `kValidDataTypes`.
         */
        static size_t CalculateDataSize(std::string_view data_type,
                                        const std::vector<std::string_view> &data);

        // Calculate instruction size based on operands
        static size_t CalculateInstructionSize(std::string_view mnemonic,
                                               const std::vector<std::string_view> &operands);

        /**
         * Determine addressing mode of an operand.
         * I.e. whether it's a register, immediate value, memory reference, or label.
         * NOTE: Not sure if this is even used?
         */
        static AddressingMode DetermineAddressingMode(std::string_view operand);
        static bool IsImmediate(std::string_view operand);
        static bool IsMemoryReference(std::string_view operand);
        static bool IsRegister(std::string_view operand);
    };

}