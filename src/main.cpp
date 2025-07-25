#include "assembler.hpp"

// lib
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

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

        cforge::Parser parser;
        auto tokens = lexer.get_tokens();

        parser.Parse(tokens);

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}