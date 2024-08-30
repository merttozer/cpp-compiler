#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType
{
    Identifier,
    Keyword,
    NumberLiteral,
    StringLiteral,
    CharacterLiteral,
    Operator,
    Separator,
    Comment,
    EndOfFile,
    Unknown,
    BooleanLiteral,
    NullLiteral,
    FloatingPointLiteral
};

class Token
{
public:
    Token(TokenType type,
          const std::string& value,
          int line,
          int column) noexcept
      : type(type)
      , value(value)
      , line(line)
      , column(column)
    {
    }

    TokenType getType() const noexcept { return type; }
    const std::string& getValue() const noexcept { return value; }
    int getLine() const noexcept { return line; }
    int getColumn() const noexcept { return column; }

    std::string toString() const noexcept
    {
        return std::string("Token(") + tokenTypeToString(type) + ", \"" +
               value + "\", Line: " + std::to_string(line) +
               ", Column: " + std::to_string(column) + ")";
    }

private:
    TokenType type;
    std::string value;
    int line;
    int column;

    static constexpr const char* tokenTypeToString(TokenType type) noexcept
    {
        switch (type) {
            case TokenType::Identifier:
                return "Identifier";
            case TokenType::Keyword:
                return "Keyword";
            case TokenType::NumberLiteral:
                return "NumberLiteral";
            case TokenType::StringLiteral:
                return "StringLiteral";
            case TokenType::CharacterLiteral:
                return "CharacterLiteral";
            case TokenType::Operator:
                return "Operator";
            case TokenType::Separator:
                return "Separator";
            case TokenType::Comment:
                return "Comment";
            case TokenType::EndOfFile:
                return "EndOfFile";
            case TokenType::Unknown:
                return "Unknown";
            case TokenType::BooleanLiteral:
                return "BooleanLiteral";
            case TokenType::NullLiteral:
                return "NullLiteral";
            case TokenType::FloatingPointLiteral:
                return "FloatingPointLiteral";
            default:
                return "Unknown";
        }
    }
};

#endif // TOKEN_H
