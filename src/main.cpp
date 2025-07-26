#include "assembler.hpp"
#include "linker.hpp"

// std
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using namespace cforge;

std::filesystem::path GetSourceFolder()
{
    // Assuming the source files are in the "src" directory relative to the executable
    return std::filesystem::path(__FILE__).parent_path();
}

int main()
{
    // Read prog.s into a string
    std::string source_file = (GetSourceFolder() / "prog.s").string();

    // Read the contents of the source file
    std::ifstream file(source_file);
    if (!file)
    {
        std::cerr << "Error opening file: " << source_file << std::endl;
        return 1;
    }

    std::string file_contents((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    file.close();

    try
    {

        cforge::Lexer lexer;
        lexer.set_source(file_contents);

        lexer.Analyze();

        Parser parser;
        auto tokens = lexer.get_tokens();

        IR ir = parser.Parse(tokens);
        std::cout << "Parsed IR version: " << ir.version << std::endl;

        // link
        Linker linker;
        std::vector<uint8_t> linked_output = linker.Link(ir);
        std::cout << "Linked output size: " << linked_output.size() << " bytes" << std::endl;
        // Write liked output
        for (const auto &byte : linked_output)
        {
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}