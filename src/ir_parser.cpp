#include "ir_parser.hpp"

// std
#include <fstream>

namespace cforge
{
    IR IrParser::Parse(std::string const &input)
    {
        char curr = '\0';
        size_t index = 0;

        auto Peek = [&input, &index]()
        {
            return index < input.size() ? input[index] : EOF;
        };
        auto Advance = [&curr, &index, &Peek]()
        {
            curr = Peek();
            if (curr != EOF)
                ++index;
            return curr;
        };

        IR ir;

        // Expect a [Metadata] section
        if (input.substr(index, 10) != "[Metadata]")
        {
            throw Error("Expected [Metadata] section", 0, "IR parsing failed");
        }

        index += 10; // Skip [Metadata]

        // Read version
        if (input.substr(index, 8) != "version=")
        {
            throw Error("Expected version declaration", 0, "IR parsing failed");
        }
        index += 8; // Skip version=
        size_t version_start = index;
        while (index < input.size() && input[index] != '\n')
        {
            ++index;
        }
        ir.version = input.substr(version_start, index - version_start); // Parse version number

        // Check if next sequence defines Section Header
        if (input.substr(index, 9) != "[Sections]")
        {
            throw Error("Expected [Sections] section", 0, "IR parsing failed");
        }
        index += 9; // Skip [Sections]

        // Read sections
        while (index < input.size())
        {
            if (input.substr(index, 1) != "[")
            {
                throw Error("Expected section header", 0, "IR parsing failed");
            }
            ++index; // Skip '['

            size_t section_start = index;
            while (index < input.size() && input[index] != ']')
            {
                ++index;
            }
            if (index >= input.size())
            {
                throw Error("Unterminated section header", 0, "IR parsing failed");
            }
            std::string section_name = input.substr(section_start, index - section_start);
            ++index; // Skip ']'

            Section section;
            section.size = 0;

            // Read section data
            while (index < input.size() && input[index] != '[')
            {
                if (input[index] == '\n')
                {
                    ++index; // Skip newline
                    continue;
                }
                section.data.push_back(static_cast<uint8_t>(input[index]));
                ++section.size;
                ++index;
            }

            ir.sections[section_name] = std::move(section);
        }

        // Check if we reached the end of input
        if (index < input.size())
        {
            throw Error("Unexpected data after sections", 0, "IR parsing failed");
        }

        return ir;
    }

    IR IrParser::ParseFile(const std::filesystem::path &path)
    {
        // Open the file and call Parse on its contents
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw Error("Failed to open file: " + path.string());
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        return Parse(content);
    }

    void IrParser::WriteToFile(const IR &ir, const std::filesystem::path &path)
    {
        // Implementation of writing IR to file goes here
        // For now, we will return true to indicate success
        std::ofstream file(path);
        if (!file.is_open())
        {
            throw Error("Failed to open file for writing: " + path.string());
        }

        // Create metadata entry
        file << "[Metadata]\n";
        file << "Version: " << ir.version << "\n";

        // Create section entry
        file << "[Sections]\n";
        for (const auto &section : ir.sections)
        {
            file << "[" << section.first << "]\n";
            file << "Size: " << section.second.size << "\n";

            for (const auto &byte : section.second.data)
            {
                file << static_cast<int>(byte) << " ";
            }

            file << "\n";
        }

        // Create Linker Metadata entry
        // Not yet implemented
        file.close();
    }
}