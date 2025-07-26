#pragma once

#include "types.hpp"
#include "error.hpp"
#include "instruction_set.hpp"
#include "ir_parser.hpp"

// std
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
            NUMBER,
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
        uint32_t line = EOF; // Primarily for logging compiler errors
        virtual ~Stmt() = default;
    };

    struct LabelStmt : public Stmt
    {
        std::string name;

        LabelStmt(std::string name) : name(name)
        {
            kind = Kind::LABEL;
        }
    };

    struct DirectiveStmt : public Stmt
    {
        std::string name;
        std::vector<std::string> args;

        DirectiveStmt(std::string name, std::vector<std::string> args)
            : name(name), args(std::move(args))
        {
            kind = Kind::DIRECTIVE;
        }
    };

    struct InstrStmt : public Stmt
    {
        std::string mnemonic;
        std::vector<std::string> operands;

        InstrStmt(std::string mnemonic, std::vector<std::string> operands)
            : mnemonic(mnemonic), operands(std::move(operands))
        {
            kind = Kind::INSTRUCTION;
        }
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
        void LexNumber();

        // Predicate-based chunk consumer
        template <typename Predicate>
        std::string_view ConsumeWhile(Predicate &&p, size_t &start);
    };

    class Parser : InstructionSet
    {
    public:
        IR Parse(
            const std::vector<Token> &tokens);

    private:
        Token &Peek();
        Token &Consume();

        // Consume tokens while `pred(peeked token)` returns true,
        // skipping over commas.
        template <typename Pred>
        std::vector<std::string> ConsumeWhileTokens(Pred &&pred);

        // Single-token statement, e.g. LabelStmt("foo")
        template <typename StmtT>
        std::unique_ptr<Stmt> MakeSingleTokenStmt();

        // Head-token + list of comma-separated tokens, e.g.
        // DirectiveStmt(".data", {"4", "8", "16"})
        template <typename StmtT>
        std::unique_ptr<Stmt> MakeListTokenStmt();

        // Dispatch helpers
        std::unique_ptr<Stmt> ParseLabelStmt();
        std::unique_ptr<Stmt> ParseDirectiveStmt();
        std::unique_ptr<Stmt> ParseInstructionStmt();

        // storage & cursor
        std::vector<Token> tokens_;
        size_t index_ = 0;

        // Section management
        std::unordered_map<std::string, size_t> section_size_map_;
        std::unordered_map<std::string, std::vector<uint8_t>> section_data_map_; // For "compiled" data
        std::string current_section_ = "";

        // Relocation & linking
        std::unordered_map<std::string, UnLocalizedOffset> symbol_map_;
        std::unordered_set<std::string> global_symbols_;
        std::vector<RelocationEntry> relocations_;
    };

}; // namespace cforge