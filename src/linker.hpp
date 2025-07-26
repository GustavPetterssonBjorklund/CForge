#pragma once

#include "error.hpp"
#include "instruction_set.hpp"
#include "ir_parser.hpp"

namespace cforge
{

    class Linker
    {
    public:
        std::vector<uint8_t>
        Link(
            const cforge::IR &ir);

    private:
        /**
         * @brief Extracts a 4-byte copy from the input data at the specified offset.
         * @param input The input data to extract from.
         * @param offset The offset to start extracting from.
         * @return A vector containing the 4-byte copy.
         */
        static std::vector<uint8_t> Extract4ByteCopy(
            const std::vector<uint8_t> &input,
            size_t offset);

        void CreateAbsoluteSectionMap(
            const IR &ir);

        void CreateAbsoluteSymbolMap(
            const IR &ir);

        /**
         * @brief Resolves a relocation entry in the input data.
         * @param ir The IR containing the relocation entries.
         * @param reloc The relocation entry to resolve.
         * @param input The input data to modify.
         * @return same reference as `input`, can thus be omitted.
         */
        std::vector<uint8_t> &ResolveRelocation(
            const IR &ir,
            const RelocationEntry &reloc,
            std::vector<uint8_t> &input);

        std::unordered_map<std::string, size_t> absolute_section_map_; // Maps to section positions after sorting / offsetting
        std::unordered_map<std::string, size_t> absolute_symbol_map_;
    };

}