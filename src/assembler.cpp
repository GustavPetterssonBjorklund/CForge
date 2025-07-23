#include "assembler.h"

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
                while (!IsAtEnd() && Advance() != '\n')
                {
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

        // Consume alumnn/_
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
            [](char c)
            { return std::isalnum(c) || c == '_'; },
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
            Advance(); // Consume the newline character
        }
        else if (curr_ == ',')
        {
            tokens_.push_back(Token{
                Token::Type::COMMA,
                std::string_view(","),
                line_});
            Advance(); // Consume the comma
        }
        else
        {
            Advance(); // Consume the character
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

            // Check for special characters first

            if (curr_ == '.')
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
                LexSpecialCharacter();
            }
        }

        for (const auto &token : tokens_)
        {
            std::cout << "Token: " << token.value << " Type: " << static_cast<int>(token.type) << " Line: " << token.line_number << std::endl;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Parser Implementation
    ///////////////////////////////////////////////////////////////////////////
    std::vector<std::unique_ptr<Stmt>> Parser::Parse(const std::vector<Token> &tokens)
    {
        size_t index = 0;
        auto Peek = [&]() -> const Token &
        {
            return tokens[index];
        };
        auto consume = [&]()
        {
            return tokens[index++];
        };

        std::vector<std::unique_ptr<Stmt>> stmts;

        while (index < tokens.size())
        {
            std::cout << "Parsing token at index " << index << ": " << Peek().value << std::endl;
            auto t = Peek();
            if (t.type == Token::Type::DIRECTIVE)
            {
                std::cout << "Directive: " << t.value << std::endl;
                auto name = consume().value;
                std::vector<std::string_view> args;
                // Collect until end of line or next directive / instruction
                while (index < tokens.size() && Peek().type != Token::Type::NEWLINE)
                {
                    std::cout << "Argument: " << Peek().value << std::endl;
                    args.push_back(consume().value);
                }
                stmts.push_back(std::make_unique<DirectiveStmt>(name, std::move(args)));
            }
            else if (t.type == Token::Type::IDENTIFIER)
            {
                std::cout << "Identifier: " << t.value << std::endl;
                // mnemonic
                auto mnemonic = consume().value;
                std::vector<std::string_view> operands;

                while (index < tokens.size() && tokens[index].type != Token::Type::NEWLINE)
                {
                    if (tokens[index].type == Token::Type::COMMA)
                    {
                        ++index;  // Skip the comma
                        continue; // Skip to next iteration to avoid adding the comma
                    }
                    operands.push_back(consume().value);
                }
                stmts.push_back(std::make_unique<InstrStmt>(mnemonic, std::move(operands)));
            }
            else
            {
                // std::cout << "No definition for token type: " << static_cast<int>(t.type) << " with value: " << t.value << std::endl;
                consume();
            }
        }
        // TODO: remove
        return std::vector<std::unique_ptr<Stmt>>{};
    }

} // namespace cforge
