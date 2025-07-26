#pragma once

#include "types.hpp"
#include "error.hpp"

// std
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <cstdint>
#include <vector>
#include <string>

// lib
#include <nlohmann/json.hpp>

namespace cforge
{
    using json = nlohmann::json;

    struct InstructionInfo
    {
        enum class Type
        {
            R_TYPE = 0x33,
            I_TYPE = 0x13,
            LOAD = 0x03,
            STORE = 0x23,
            BRANCH = 0x63,
            U_TYPE = 0x37,
            J_TYPE = 0x6F,

            // Impossible types [0x80:0xFF] (> 7 bit)
            NONE = 0xFF,
            PSEUDO = 0xFE,
        };

        Type opcode;

        uint8_t func3;
        uint8_t func7;

        uint8_t operand_count;
    };

    struct CompiledInstruction
    {
        std::vector<uint8_t> bytes;               // Compiled instruction bytes
        std::vector<RelocationEntry> relocations; // Relocation entries for linking
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

        /**
         * @brief Get the register code for a given register name.
         * @param reg The register name (e.g., "x0", "x1", etc.).
         * @note also supports register aliases like "zero", "ra", etc.
         * @return The register code (0-31) for the given register.
         */
        static uint8_t GetRegisterCode(std::string_view reg);

        /**
         * @brief Get the bytes for a data type and its values.
         * @param data_type Type of data, must be one of the valid data types.
         * @param data Values to convert to bytes.
         * @return Vector of bytes representing the data.
         */
        static std::vector<uint8_t> GetDataBytes(
            const std::string data_type,
            const std::vector<std::string> &data);

        /**
         * @brief Compiles an instruction into its bytecode representation.
         * @attention This method makes some relocations for labels.
         * @param mnemonic The mnemonic of the instruction.
         * @param operands The operands for the instruction.
         * @param line The line number in the source code (for error reporting).
         * @return A vector of bytes representing the compiled instruction.
         * @note If the instruction requires expansion to 8 bytes, this method will
         * return 8-bytes instead of 4.
         */
        static CompiledInstruction CompileInstruction(
            std::string mnemonic,
            const std::vector<std::string> &operands,
            uint32_t line = 0);

        /**
         * @brief Calculates the size of data based on type and amount of values
         * @attention This method will return UB if data type is invalid
         * @param data_type Type of data MUST be one of the valid data types
         * as defined in `kValidDataTypes`.
         */
        static size_t CalculateDataSize(std::string_view data_type,
                                        const std::vector<std::string> &data);

        /**
         * @brief Calculates the size of an instruction based on it's mnemonic and operands.
         * @attention This method is def broken for expanding instructions
         */
        static size_t CalculateInstructionSize(std::string mnemonic,
                                               const std::vector<std::string> &operands);

    private:
        static CompiledInstruction CompileRTypeInstruction(
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
        static CompiledInstruction CompileITypeInstruction(
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
        static CompiledInstruction CompileLoadStoreInstruction(
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
        static CompiledInstruction CompileBranchInstruction(
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
        static CompiledInstruction CompileUTypeInstruction(
            const InstructionInfo *info,
            const std::vector<std::string_view> &operands);
        static CompiledInstruction CompileJTypeInstruction(
            const size_t &instruction_id,
            const std::string mnemonic,
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
        static CompiledInstruction CompilePseudoInstruction(
            size_t &instruction_id,
            const std::string mnemonic,
            const InstructionInfo *info,
            const std::vector<std::string> &operands);
    };
}