#pragma once

// lib
#include <string>
#include <vector>
#include <string_view>
#include <functional>
#include <memory>

namespace cforge
{
    // Tokenization
    struct Token
    {
        enum class Type
        {
            IDENTIFIER,
            DIRECTIVE,
            LABEL,
            NEWLINE,
            COMMA,
        } type;

        std::string_view value;
        size_t line_number;
    };

    // IR
    struct Stmt
    {
        enum class Kind
        {
            LABEL,
            DIRECTIVE,
            INSTRUCTION,
        } kind;
        virtual ~Stmt() = default;
    };

    struct LabelStmt : public Stmt
    {
        std::string_view name;

        LabelStmt(std::string_view name) : name(name)
        {
            kind = Kind::LABEL;
        }
    };

    struct DirectiveStmt : public Stmt
    {
        std::string_view name;
        std::vector<std::string_view> args;

        DirectiveStmt(std::string_view name, std::vector<std::string_view> args)
            : name(name), args(std::move(args))
        {
            kind = Kind::DIRECTIVE;
        }
    };

    struct InstrStmt : public Stmt
    {
        std::string_view mnemonic;
        std::vector<std::string_view> operands;

        InstrStmt(std::string_view mnemonic, std::vector<std::string_view> operands)
            : mnemonic(mnemonic), operands(std::move(operands))
        {
            kind = Kind::INSTRUCTION;
        }
    };

    struct Line
    {
        size_t line_number;
        std::vector<Token> tokens;

        Line(size_t line_num) : line_number(line_num) {}
    };

    class Lexer
    {
    public:
        Lexer() = default;
        ~Lexer() = default;

        void set_source(const std::string &file) { source_ = file; }
        const std::string &get_source() const { return source_; }

        // Runs the tokenization process
        void Analyze();

        // Token Getter
        const std::vector<Token> &get_tokens() const { return tokens_; }

    private:
        std::string source_;

        size_t pos_ = 0;
        size_t line_ = 1;  // 1-based line number
        char curr_ = '\0'; // Current character

        std::vector<Token> tokens_;

        // Advance / Peek functions
        char Peek() const;
        char Advance();
        bool IsAtEnd() const;

        // High-level phases
        void Tokenize();
        void SkipWhitespaceAndComments();

        // Per-Token lexers
        void LexDirective();
        void LexLabelOrIdentifier();
        void LexSpecialCharacter();

        // Predicate-based chunk consumer
        template <typename Predicate>
        std::string_view ConsumeWhile(Predicate &&p, size_t &start);
    };

    class Parser
    {
    public:
        Parser() = default;
        ~Parser() = default;

        std::vector<std::unique_ptr<Stmt>> Parse(const std::vector<Token> &tokens);
    };

    class Assembler
    {
    public:
        Assembler() = default;
        ~Assembler() = default;
        Assembler(const Assembler &) = delete;

    private:
        std::string source_file_;
    };
} // namespace cforge.assembler