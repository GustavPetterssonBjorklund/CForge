#pragma once

#include <exception>
#include <cstdio>
#include <cstddef>
#include <string_view>

class Error : public std::exception
{
public:
    // max bytes for the formatted message (including NUL)
    static constexpr std::size_t BUF_SIZE = 512;

    // Generic error
    explicit Error(std::string_view message) noexcept
    {
        int n = std::snprintf(
            buf_, BUF_SIZE,
            "\033[31mError: %.*s\033[0m",
            int(message.size()), message.data());
        // on snprintf error just produce an empty string
        if (n < 0)
            buf_[0] = '\0';
    }

    // Error at a token/line, optional details
    explicit Error(std::string_view token,
                   std::size_t line,
                   std::string_view details = {}) noexcept
    {
        if (details.empty())
        {
            std::snprintf(
                buf_, BUF_SIZE,
                "\033[31mError at line %zu: %.*s\033[0m",
                line,
                int(token.size()), token.data());
        }
        else
        {
            std::snprintf(
                buf_, BUF_SIZE,
                "\033[31mError at line %zu: %.*s - %.*s\033[0m",
                line,
                int(token.size()), token.data(),
                int(details.size()), details.data());
        }
    }

    // what() must be noexcept
    const char *what() const noexcept override
    {
        return buf_;
    }

private:
    char buf_[BUF_SIZE];
};
