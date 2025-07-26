#pragma once

#include "types.hpp"
#include "error.hpp"
#include "instruction_set.hpp"

// lib
#include <nlohmann/json.hpp>

// std
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace cforge
{

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
		static void WriteToFile(const IR &ir, const std::filesystem::path &path);

	private:
	};
}