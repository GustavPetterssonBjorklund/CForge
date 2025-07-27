#include "ir_parser.hpp"

// std
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace cforge
{
    IR IrParser::Parse(std::string const &input)
    {
        return IR{};
    }

    IR IrParser::ParseFile(const std::filesystem::path &path)
    {
        return IR{};
    }

    void IrParser::WriteToFile(const IR &ir, const std::filesystem::path &path)
    {
        json j;
        j["version"] = ir.version;
        for (const auto &section_pair : ir.section_size_map)
        {
            const auto &section_name = section_pair.first;
            const auto &section_size = section_pair.second;

            // Get data
            const std::vector<uint8_t> &section_data = ir.section_data.find(section_name) != ir.section_data.end()
                                                           ? ir.section_data.at(section_name)
                                                           : std::vector<uint8_t>{};

            j["sections"] += {
                {"name", section_name},
                {"size", section_size},
                {"data", section_data}};

            j["relocations"] = json::array();
            for (const auto &reloc : ir.relocations)
            {
                j["relocations"].push_back({
                    {"type", static_cast<int>(reloc.type)},
                    {"section", reloc.section},
                    {"instruction_id", reloc.instruction_id},
                    {"symbol", reloc.symbol},
                });
            }

            j["symbols"] = json::array();
            for (const auto &symbol_pair : ir.symbol_map)
            {
                j["symbols"].push_back({
                    {"name", symbol_pair.first},
                    {"section", symbol_pair.second.section},
                    {"offset", symbol_pair.second.offset},
                });
            }
        }
        std::ofstream file(path);
        if (!file.is_open())
        {
            throw Error("Failed to open file for writing: " + path.string());
        }

        std::cout << "Writing IR to file: " << path << std::endl;

        file << j.dump(4);
        file.close();
    }

} // namespace cforge