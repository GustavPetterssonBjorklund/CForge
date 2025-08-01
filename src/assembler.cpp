#include "assembler.hpp"

// lib
#include <iostream>

namespace cforge
{
    ///////////////////////////////////////////////////////////////////////////
    /// Lexer Implementation
    ///////////////////////////////////////////////////////////////////////////

    void Lexer::Analyze()
    {
        pos_ = 0;
        line_ = 1;
        curr_ = '\0';
        tokens_.clear();
        Tokenize();
    }

    char Lexer::Peek() const
    {
        return pos_ < source_.size() ? source_[pos_] : EOF;
    }

    char Lexer::Advance()
    {
        curr_ = Peek();
        ++pos_;
        if (curr_ == '\n')
        {
            ++line_;
        }
        return curr_;
    }

    bool Lexer::IsAtEnd() const
    {
        return pos_ >= source_.size();
    }

    /**
     * Consumes characters from the source string while the predicate `p` returns true.
     */
    template <typename Predicate>
    std::string_view Lexer::ConsumeWhile(Predicate &&p, size_t &start)
    {
        while (!IsAtEnd() && p(Peek()))
        {
            Advance();
        }
        return std::string_view(source_.data() + start, pos_ - start);
    }

    void Lexer::SkipWhitespaceAndComments()
    {
        while (!IsAtEnd())
        {
            if (Peek() == ' ' || Peek() == '\t')
            {
                Advance();
            }
            else if (Peek() == '#')
            {
                // skip until end of line
                while (!IsAtEnd() && Peek() != '\n')
                {
                    Advance();
                }
            }
            else
            {
                break; // Found a non-whitespace, non-comment character
            }
        }
    }

    void Lexer::LexDirective()
    {
        // '.' already consumed
        size_t start = pos_ - 1; // Start at the current position minus the consumed '.'

        // Consume alnum/_
        auto view = ConsumeWhile(
            [](char c)
            { return std::isalnum(c) || c == '_'; },
            start);

        tokens_.push_back(Token{
            Token::Type::DIRECTIVE,
            view,
            line_});
    }

    void Lexer::LexLabelOrIdentifier()
    {

        size_t start = pos_;
        // Consume alphanumeric characters and underscores
        auto view = ConsumeWhile(
            // in accordance with cppreference, it is unsafe to run alnum() on char directly
            [](char c)
            { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; },
            start);

        if (Peek() == ':')
        {
            Advance(); // Consume the ':'
            tokens_.push_back(Token{
                Token::Type::LABEL,
                view,
                line_});
        }
        else
        {
            tokens_.push_back(Token{
                Token::Type::IDENTIFIER,
                view,
                line_});
        }
    }

    void Lexer::LexSpecialCharacter()
    {
        // Check for newline character
        if (curr_ == '\n')
        {
            tokens_.push_back(Token{
                Token::Type::NEWLINE,
                std::string_view("\n"),
                line_});
            return;
        }
        else if (curr_ == ',')
        {
            tokens_.push_back(Token{
                Token::Type::COMMA,
                std::string_view(","),
                line_});
            return;
        }
    }

    /**
     * Creates a token from a number, can be of type bin, hex or decimal.
     */
    void Lexer::LexNumber()
    {
        size_t start = pos_;

        // Consume digits and optional prefix
        auto view = ConsumeWhile(
            [](char c)
            {
                unsigned char uc = static_cast<unsigned char>(c);
                return std::isdigit(uc) || (uc >= 'a' && uc <= 'f') || (uc >= 'A' && uc <= 'F') || uc == 'x' || uc == 'b' || uc == 'X' || uc == 'B';
            },
            start);

        if (!view.empty())
        {
            tokens_.push_back(Token{
                Token::Type::NUMBER,
                view,
                line_});
        }
    }

    /**
     * Tokenizes the source string by processing characters one by one.
     * It identifies directives, identifiers, and other token types.
     */
    void Lexer::Tokenize()
    {

        while (!IsAtEnd())
        {

            SkipWhitespaceAndComments();

            if (IsAtEnd())
                break;

            curr_ = Peek();

            LexSpecialCharacter();

            if (isdigit(static_cast<unsigned char>(curr_)))
            {
                LexNumber();
            }
            else if (curr_ == '.')
            {
                Advance(); // Consume the '.'
                LexDirective();
            }
            else if (std::isalpha(curr_) || curr_ == '_')
            {
                LexLabelOrIdentifier();
            }
            else
            {
                Advance();
            }
        }

        // Print all tokens for debugging
        for (const auto &token : tokens_)
        {
            std::cout << "Token: " << token.value << " (Type: "
                      << static_cast<int>(token.type) << ", Line: "
                      << token.line_number << ")\n";
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Parser Implementation
    ///////////////////////////////////////////////////////////////////////////

    Token &Parser::Peek()
    {
        if (index_ >= tokens_.size())
        {
            throw Error("Unexpected end of input", 0, "No more tokens available");
        }
        return tokens_[index_];
    }
    Token &Parser::Consume()
    {
        if (index_ >= tokens_.size())
        {
            throw Error("Unexpected end of input", 0, "No more tokens available");
        }
        return tokens_[index_++];
    }

    template <typename Pred>
    std::vector<std::string> Parser::ConsumeWhileTokens(Pred &&pred)
    {
        std::vector<std::string> items;
        while (index_ < tokens_.size() && pred(Peek()))
        {
            if (Peek().type == Token::Type::COMMA)
            {
                Consume();
            }
            else
            {
                items.push_back(std::string(Consume().value));
            }
        }
        return items;
    }

    template <typename StmtT>
    std::unique_ptr<Stmt> Parser::MakeSingleTokenStmt()
    {
        Token t = Consume();
        auto ptr = std::make_unique<StmtT>(std::string(t.value));
        ptr->line = t.line_number;
        return ptr;
    }

    template <typename StmtT>
    std::unique_ptr<Stmt> Parser::MakeListTokenStmt()
    {
        Token head = Consume();
        // stop on NEWLINE
        std::vector<std::string> args = ConsumeWhileTokens(
            [](auto const &tk)
            {
                return tk.type != Token::Type::NEWLINE;
            });
        auto ptr = std::make_unique<StmtT>(std::string(head.value), std::move(args));
        ptr->line = head.line_number;
        return ptr;
    }

    std::unique_ptr<Stmt> Parser::ParseLabelStmt()
    {
        std::unique_ptr<Stmt> ptr = MakeSingleTokenStmt<LabelStmt>();

        // Must be inside a section
        if (current_section_.empty())
        {
            throw Error("Label used outside of a section", ptr->line);
        }

        // Add the label to the symbol map
        std::string name = static_cast<LabelStmt *>(ptr.get())->name;
        if (symbol_map_.find(name) != symbol_map_.end())
        {
            throw Error(std::string(name), ptr->line, "Label defined multiple times");
        }
        auto offset = UnLocalizedOffset(
            current_section_,
            section_size_map_[current_section_]);
        symbol_map_[name] = offset;

        return ptr;
    }

    std::unique_ptr<Stmt> Parser::ParseDirectiveStmt()
    {
        auto ptr = MakeListTokenStmt<DirectiveStmt>();
        auto *d = static_cast<DirectiveStmt *>(ptr.get());
        std::string directive_name(d->name);
        if (!InstructionSet::IsValidDirective(d->name))
        {
            throw Error(directive_name, d->line, "Invalid directive");
        }

        // FIX: Remember to handle errors for:
        // - globl label not defined
        // - multiple equal definitions for globl labels
        if (directive_name == ".globl")
        {
            // Handle global directive
            if (d->args.size() != 1)
            {
                throw Error(directive_name, d->line, "Expected exactly one argument for .globl directive");
            }
            auto label = d->args[0];
            global_symbols_.insert(std::string(label));
        }
        else if (directive_name == ".align")
        {
            std::cout << "Aligning section with .align directive\n";
            // Expect one or two arguments
            if (d->args.size() == 1)
            {
                try
                {
                    // Get alignment value
                    int alignment_n = std::stoi(d->args[0]);

                    if (alignment_n <= 0 || alignment_n >= 32)
                    {
                        throw Error(directive_name, d->line, "Alignment must be a positive integer less than 32");
                    }

                    // Ensure we're in a section
                    if (current_section_.empty())
                    {
                        throw Error("Align directive used outside of a section", d->line);
                    }

                    // Calculate required padding
                    size_t current_size = section_size_map_[current_section_];
                    size_t alignment = 1 << alignment_n; // 2^alignment_n
                    size_t padding = (alignment - (current_size % alignment)) % alignment;

                    std::cout << "Aligning section '" << current_section_ << "' by " << padding << " bytes\n";
                    // Update section size and insert padding
                    section_size_map_[current_section_] += padding;
                    section_data_map_[current_section_].insert(
                        section_data_map_[current_section_].end(),
                        padding, 0); // Fill with zeros
                }
                catch (const std::exception &e)
                {
                    throw Error(directive_name, d->line, e.what());
                }
            }
            else if (d->args.size() == 2)
            {
                try
                {
                    // Get alignment value
                    int alignment_n = std::stoi(d->args[0]);

                    // Get fill value, default to 0
                    uint8_t fill_value = 0;
                    if (!d->args[1].empty())
                    {
                        fill_value = static_cast<uint8_t>(std::stoi(d->args[1]));
                    }

                    // Validate alignment_n
                    if (alignment_n <= 0 || alignment_n >= 32)
                    {
                        throw Error(directive_name, d->line, "Alignment must be a positive integer less than 32");
                    }

                    // Validate fill value
                    if (fill_value > 255 || fill_value < 0)
                    {
                        throw Error(directive_name, d->line, "Fill value must be in range [0, 255]");
                    }

                    // Ensure we're in a section
                    if (current_section_.empty())
                    {
                        throw Error("Align directive used outside of a section", d->line);
                    }

                    // Calculate required padding
                    size_t current_size = section_size_map_[current_section_];
                    size_t alignment = 1 << alignment_n; // 2^alignment_n
                    size_t padding = (alignment - (current_size % alignment)) % alignment;

                    // Update section size and insert padding
                    section_size_map_[current_section_] += padding;
                    section_data_map_[current_section_].insert(
                        section_data_map_[current_section_].end(),
                        padding, fill_value); // Fill with specified value
                }
                catch (const std::exception &e)
                {
                    throw Error(directive_name, d->line, e.what());
                }
            }
            else
            {
                throw Error(directive_name, d->line, ".align directive must be in the form \".align <N> [, <fill>]\"");
            }
        }
        // Check for .section directive
        else if (directive_name == ".section")
        {
            if (d->args.size() != 1)
            {
                throw Error(directive_name, d->line, "Expected exactly one argument for .section directive");
            }
            std::string section_name = d->args[0];

            // Switch to the next section while safely initializing `section_map_`
            if (section_size_map_.find(section_name) == section_size_map_.end())
            {
                section_size_map_[section_name] = 0; // Initialize section size to 0
            }
            current_section_ = section_name;
        }
        else if (directive_name == ".space")
        {
            if (d->args.size() != 1)
            {
                throw Error(directive_name, d->line, "Expected exactly one argument for .space directive");
            }
            // Convert the argument to a size_t
            size_t space_size = std::stoul(std::string(d->args[0]));
            // Make sure we're in a valid section for .space
            if (current_section_.empty())
            {
                throw Error("Space directive used outside of a section", d->line);
            }
            // Update the section size map
            section_size_map_[current_section_] += space_size;
            // Initialize the section data with zeros
            section_data_map_[current_section_].resize(
                section_data_map_[current_section_].size() + space_size, 0);
        }
        // Check for valid data directive
        else if (IsValidDataType(d->name))
        {
            // make sure the data directive exists in a valid section
            if (!IsValidDataTypeSection(current_section_))
            {
                throw Error(directive_name, d->line, "Data directive used in invalid section");
            }

            // Calculate the size of the data
            size_t data_size = InstructionSet::CalculateDataSize(d->name, d->args);
            section_size_map_[current_section_] += data_size;

            // Convert data to bytes
            std::vector<uint8_t> data_bytes = InstructionSet::GetDataBytes(d->name, d->args);

            // Store the data in the section data map
            section_data_map_[current_section_].insert(
                section_data_map_[current_section_].end(),
                data_bytes.begin(),
                data_bytes.end());
        }
        else
        {

            throw Error(directive_name, d->line, "Directive is valid but not handled by the assembler");
        }

        return ptr;
    }

    std::unique_ptr<Stmt> Parser::ParseInstructionStmt()
    {
        std::unique_ptr<Stmt> ptr = MakeListTokenStmt<InstrStmt>();
        // Make sure the instruction lives in a valid section
        if (current_section_ != ".text")
        {
            throw Error("Instruction used in non \".text\" section", ptr->line);
        }

        std::string mnemonic =
            static_cast<InstrStmt *>(ptr.get())->mnemonic;
        std::vector<std::string> operands =
            static_cast<InstrStmt *>(ptr.get())->operands;

        size_t instruction_size = CalculateInstructionSize(
            mnemonic,
            operands);

        // Will always be .text section*, but this is consistent
        section_size_map_[current_section_] += instruction_size;

        // Compile the instruction
        CompiledInstruction compiled = InstructionSet::CompileInstruction(
            mnemonic,
            operands,
            ptr->line);

        // Store the compiled instruction in the section data map
        auto &section_data = section_data_map_[std::string(current_section_)];
        section_data.insert(
            section_data.end(),
            compiled.bytes.begin(),
            compiled.bytes.end());

        // Add relocations if any
        for (const auto &reloc : compiled.relocations)
        {
            relocations_.push_back(reloc);
        }

        return ptr;
    }

    IR Parser::Parse(
        const std::vector<Token> &tokens)
    {
        tokens_ = tokens;
        index_ = 0;

        // Reset section management
        section_size_map_.clear();
        current_section_ = "";
        relocations_.clear();

        std::vector<std::unique_ptr<Stmt>> stmts;
        while (index_ < tokens_.size())
        {
            // skip blank lines
            while (index_ < tokens_.size() &&
                   Peek().type == Token::Type::NEWLINE)
            {
                Consume();
            }
            if (index_ >= tokens_.size())
                break;

            Token const tok = Peek();
            std::unique_ptr<Stmt> stmt;
            switch (tok.type)
            {
            case Token::Type::LABEL:
                stmt = ParseLabelStmt();
                break;
            case Token::Type::DIRECTIVE:
                stmt = ParseDirectiveStmt();
                break;
            case Token::Type::IDENTIFIER:
                stmt = ParseInstructionStmt();
                break;
            default:
                throw Error("Unexpected token '" + std::string(tok.value) + "'",
                            tok.line_number,
                            "Expected label, directive or instruction");
            }

            // consume a trailing newline if present
            if (index_ < tokens_.size() &&
                Peek().type == Token::Type::NEWLINE)
            {
                Consume();
            }
            stmts.push_back(std::move(stmt));
        }

        // Print all parsed statements for debugging
        for (const auto &stmt : stmts)
        {
            switch (stmt->kind)
            {
            case Stmt::Kind::LABEL:
                std::cout << "Parsed Label: " << static_cast<LabelStmt *>(stmt.get())->name << "\n";
                break;
            case Stmt::Kind::DIRECTIVE:
                std::cout << "Parsed Directive: " << static_cast<DirectiveStmt *>(stmt.get())->name << "\n";
                for (const auto &arg : static_cast<DirectiveStmt *>(stmt.get())->args)
                {
                    std::cout << "  Arg: " << arg << "\n";
                }
                break;
            case Stmt::Kind::INSTRUCTION:
                std::cout << "Parsed Instruction: " << static_cast<InstrStmt *>(stmt.get())->mnemonic << "\n";
                for (const auto &arg : static_cast<InstrStmt *>(stmt.get())->operands)
                {
                    std::cout << "  Operand: " << arg << "\n";
                }
                break;
            }
        }

        // Print section sizes
        std::cout << "Section sizes:\n";
        for (const auto &section : section_size_map_)
        {
            std::cout << "  " << section.first << ": " << section.second << " bytes\n";
        }

        // Print symbol map
        std::cout << "Symbol map:\n";
        for (const auto &symbol : symbol_map_)
        {
            std::cout << "  " << symbol.first << ": "
                      << symbol.second.section << " at offset "
                      << symbol.second.offset << "\n";
        }

        // Print global symbols
        std::cout << "Global symbols:\n";
        for (const auto &symbol : global_symbols_)
        {
            std::cout << "  " << symbol << "\n";
        }

        // Print raw data in sections
        std::cout << "Section data:\n";
        for (const auto &section : section_data_map_)
        {
            std::cout << "  " << section.first << ": ";
            for (const auto &byte : section.second)
            {
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << "\n";
        }

        // Create ir object
        IR ir;
        ir.version = "1.1";                                 // Set the IR version
        ir.section_size_map = std::move(section_size_map_); // Move section sizes to IR
        ir.section_data = std::move(section_data_map_);     // Move section data to IR
        ir.symbol_map = std::move(symbol_map_);             // Move symbol map to IR
        ir.relocations = std::move(relocations_);           // Move relocations to IR
        return ir;
    }

} // namespace cforge