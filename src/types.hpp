#pragma once

// std
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>

namespace cforge
{

	/**
	 * @brief Represents a "unlocalized" offset by storing both section in-section-offset.
	 * @note Primarily used for linking and relocation purposes.
	 */
	struct UnLocalizedOffset
	{
		std::string section; // Section name
		size_t offset;		 // Offset in the section
		UnLocalizedOffset(std::string sec, size_t off)
			: section(sec), offset(off) {}
		UnLocalizedOffset() = default;
	};

	/**
	 * @brief Represents a relocation entry for linking.
	 * This entry contains information about how to resolve a symbol
	 * @param type The type of relocation (e.g., R_RISC_V_HI20, R_RISC_V_LO12_I, etc.)
	 * @param section The section name where the relocation is applied
	 * @param instruction_id The ID of the instruction that requires relocation
	 * @param symbol The symbol to resolve
	 */
	struct RelocationEntry
	{
		enum class Type
		{
			R_RISC_V_HI20,	 // High 20-bit for "lui", "auipc"
			R_RISC_V_LO12_I, // Low 12-bit for "addi"
			R_RISC_V_LO12_S, // Low 12-bit for "sw", "sh", "sb"
			R_RISC_V_JAL,	 // JAL label relocation
		} type;

		std::string section; // Section name
		size_t instruction_id;

		std::string symbol; // Symbol to resolve
	};

	/**
	 * @brief Represents the Intermediate Representation (IR) of the program.
	 * This structure contains all the necessary information for linking and final code generation.
	 */
	struct IR
	{
		/**
		 * The version of the IR format
		 */
		std::string version;

		/**
		 * Sections in the IR
		 */
		std::unordered_map<std::string, size_t> section_size_map;

		/**
		 * Data in each section in the IR
		 */
		std::unordered_map<std::string, std::vector<uint8_t>> section_data;

		/**
		 * Symbol map for linking.
		 */
		std::unordered_map<std::string, UnLocalizedOffset> symbol_map;
		/**
		 * Relocation entries for linking
		 */
		std::vector<RelocationEntry> relocations;
	};

} // cforge
