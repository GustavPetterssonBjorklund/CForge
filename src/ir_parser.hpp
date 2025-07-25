#pragma once

#include "error.hpp"

// std
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace cforge
{
	enum class IrParserFlags
	{

	};

	enum class TargetArchitecture
	{
		RISC_V, // Only currently supported architecture
	};

	struct Metadata
	{
		std::string version; // Version of the IR
	};

	struct Section
	{
		size_t size;
		std::vector<uint8_t> data;
	};

	/**
	 * Object containing all the information that can / will be stored in a file
	 * @note File structure is done in the following way:
	 * [Metadata] (all metadata is defined a string)
	 * version="<version>""
	 * [Sections]
	 * [.text]
	 * <data>
	 * [.data]
	 * <data>
	 * [.bss]
	 * <data>
	 * [Linker Metadata]
	 * [.globl]
	 * <global symbols>
	 * [.rel]
	 * <relocation entries>
	 */
	struct IR
	{
		/**
		 * The version of the IR format
		 */
		std::string version;

		/**
		 * Flags
		 */
		IrParserFlags flags;
		/**
		 * Target architecture
		 */
		TargetArchitecture target_architecture;

		/**
		 * Sections in the IR
		 */
		std::unordered_map<std::string, Section> sections;
	};

	class IrParser
	{
	public:
		IrParser() = default;

		/**
		 * Parses the input and returns an IR object.
		 * @param input The input to parse.
		 * @return The parsed IR object.
		 */
		IR Parse(const std::string &input);

		/**
		 * Parses a file and returns an IR object.
		 * @attention This function simply opens the file and calls Parse on its contents.
		 * @param path The path to the file to parse.
		 * @return The parsed IR object.
		 */
		IR ParseFile(const std::filesystem::path &path);

		/**
		 * Creates a .cir file from the IR object.
		 * @param ir The IR object to write to file.
		 * @param path The path to write the file to.
		 * @return True if the file was written successfully, false otherwise.
		 */
		void WriteToFile(const IR &ir, const std::filesystem::path &path);

	private:
	};
}