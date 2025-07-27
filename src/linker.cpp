#include "linker.hpp"

// std
#include <iostream>
#include <vector>
#include <bitset>

namespace cforge
{

    std::vector<uint8_t> Linker::Link(const IR &ir)
    {
        std::vector<uint8_t> output;                    // Should write to file instead
        output.reserve(ir.section_size_map.size() * 4); // Reserve space for the output

        // Create absolute section map
        CreateAbsoluteSectionMap(ir);

        // Resolve absolute symbols
        CreateAbsoluteSymbolMap(ir);
        std::cout << "Symbol address map:\n";
        for (const auto &symbol : absolute_symbol_map_)
        {
            std::cout << "Symbol: " << symbol.first << " Address: " << std::hex << symbol.second << "\n";
        }

        // Iterate over sections and write data
        for (const auto &section : ir.section_data)
        {
            const auto &section_name = section.first;
            const auto &section_data = section.second;

            std::cout << "\nSection: " << section_name << " Data: ";
            for (const auto &byte : section_data)
            {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            output.insert(output.end(), section_data.begin(), section_data.end());
        }

        // Resolve relocations
        for (const auto &reloc : ir.relocations)
        {
            // Get instruction [instruction_id*4:instruction_id*4 + 3]
            size_t offset = reloc.instruction_id * 4;
            std::vector<uint8_t> instruction_copy = Extract4ByteCopy(ir.section_data.at(reloc.section), offset);

            std::cout << "Old instruction bytes: ";
            for (const auto &byte : instruction_copy)
            {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::endl;
            // instruction_copy = is technically unnecessary, but it makes the code more readable
            instruction_copy = ResolveRelocation(ir, reloc, instruction_copy);

            std::cout << "\nNew instruction bytes: ";
            for (const auto &byte : instruction_copy)
            {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }

            // Write the modified instruction back to the output
            auto output_offset = absolute_section_map_.at(reloc.section) + offset;
            for (size_t i = 0; i < 4; ++i)
            {
                if (output_offset + i < output.size())
                {
                    output[output_offset + i] = instruction_copy[i];
                }
                else
                {
                    throw Error("Offset out of bounds while writing modified instruction");
                }
            }
        }

        return output;
    }

    std::vector<uint8_t> Linker::Extract4ByteCopy(
        const std::vector<uint8_t> &input,
        size_t offset)
    {
        if (offset + 4 > input.size())
        {
            throw Error("Offset out of bounds for 4-byte copy extraction, likely a bug in the linker");
        }
        return std::vector<uint8_t>(input.begin() + offset, input.begin() + offset + 4);
    }

    void Linker::CreateAbsoluteSectionMap(
        const IR &ir)
    {
        absolute_section_map_.clear();
        size_t current_offset = 0;
        for (const auto &section_pair : ir.section_size_map)
        {
            const auto &section_name = section_pair.first;
            const auto &section_size = section_pair.second;
            absolute_section_map_[section_name] = current_offset;
            current_offset += section_size;
        }
    }

    void Linker::CreateAbsoluteSymbolMap(
        const IR &ir)
    {
        absolute_symbol_map_.clear();
        for (const auto &symbol_pair : ir.symbol_map)
        {
            // Get symbol, section and offset
            const auto &symbol = symbol_pair.first;

            const auto &section = symbol_pair.second.section;
            const auto &offset_local = symbol_pair.second.offset;

            // Get the absolute section offset
            auto section_it = absolute_section_map_.find(section);
            if (section_it == absolute_section_map_.end())
            {
                throw Error("Section not found in absolute section map: " + section);
            }
            size_t section_offset = section_it->second;

            // Calculate the absolute offset
            size_t absolute_offset = section_offset + offset_local;
            absolute_symbol_map_[symbol] = absolute_offset;
        }
    }

    std::vector<uint8_t> &Linker::ResolveRelocation(
        const IR &ir,
        const RelocationEntry &reloc,
        std::vector<uint8_t> &input)
    {
        switch (reloc.type)
        {
        // case RelocationEntry::Type::R_RISC_V_HI20:
        case RelocationEntry::Type::R_RISC_V_LO12_I:
            return input;
            // case RelocationEntry::Type::R_RISC_V_LO12_S:

        case RelocationEntry::Type::R_RISC_V_JAL:
        {
            // Get the symbol address
            auto symbol_it = absolute_symbol_map_.find(reloc.symbol);
            if (symbol_it == absolute_symbol_map_.end())
            {
                throw Error("Symbol not found in absolute symbol map: " + reloc.symbol);
            }
            size_t symbol_address = symbol_it->second;

            size_t instruction_address = reloc.instruction_id * 4 + absolute_section_map_.at(reloc.section);

            size_t offset = symbol_address - instruction_address;

            // FIX: Make sure address is not larger than 20 bits?
            offset <<= 12; // Shift left by 12 bits to get the JAL offset

            // Write the symbol address into the instruction
            for (size_t i = 0; i < 4; ++i)
            {
                uint8_t &byte = input[i];
                byte |= (offset >> (i * 8)) & 0xFF;
            }

            return input;
        }

        default:
            throw Error("Unsupported relocation symbol: " +
                        reloc.symbol +
                        " NOTE: This is likely a bug in the linker");
        }
    }
}
