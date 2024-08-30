#include "Lexer.hpp"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

Lexer::Lexer() = default;

void Lexer::setSource(const std::string& source)
{
    this->source = source;
    this->index = 0;
    this->line = 1;
    this->column = 1;
}

char Lexer::currentChar() const noexcept
{
    if (index < source.size()) {
        return source[index];
    }
    return '\0';
}

char Lexer::peekChar(int offset) const noexcept
{
    size_t peekIndex = index + offset;
    if (peekIndex < source.size()) {
        return source[peekIndex];
    }
    return '\0';
}

void Lexer::advance() noexcept
{
    if (index < source.size()) {
        if (source[index] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
        ++index;
    }
}

void Lexer::skipWhitespace() noexcept
{
    while (std::isspace(currentChar())) {
        advance();
    }
}

void Lexer::skipComment()
{
    if (currentChar() == '/' && peekChar() == '/') {
        while (currentChar() != '\n' && currentChar() != '\0') {
            advance();
        }
        advance(); // Skip the newline
    } else if (currentChar() == '/' && peekChar() == '*') {
        advance(); // Skip '/'
        advance(); // Skip '*'
        while (!(currentChar() == '*' && peekChar() == '/')) {
            advance();
            if (currentChar() == '\0') {
                break;
            }
        }
        advance(); // Skip '*'
        advance(); // Skip '/'
    }
}

Token Lexer::number()
{
    std::string value;
    bool isFloatingPoint = false;

    int tokenColumn = column; // Capture the column where the number starts

    while (std::isdigit(currentChar()) || currentChar() == '.') {
        if (currentChar() == '.') {
            if (isFloatingPoint) {
                break; // Only one '.' allowed
            }
            isFloatingPoint = true;
        }
        value += currentChar();
        advance();
    }

    if (isFloatingPoint) {
        return Token(TokenType::FloatingPointLiteral, value, line, tokenColumn);
    } else {
        return Token(TokenType::NumberLiteral, value, line, tokenColumn);
    }
}

Token Lexer::identifierOrKeyword()
{
    std::string value;
    int tokenColumn = column;

    while (std::isalnum(currentChar()) || currentChar() == '_') {
        value += currentChar();
        advance();

        if (currentChar() == ':' && peekChar() == ':') {
            value += "::";
            advance(); // Skip the first ':'
            advance(); // Skip the second ':'
        }
    }

    static const std::unordered_map<std::string_view, TokenType> keywords{
        { "true", TokenType::BooleanLiteral },
        { "false", TokenType::BooleanLiteral },
        { "nullptr", TokenType::NullLiteral },
        { "int", TokenType::Keyword },
        { "return", TokenType::Keyword },
        { "if", TokenType::Keyword },
        { "else", TokenType::Keyword },
        { "for", TokenType::Keyword },
        { "while", TokenType::Keyword },
        { "float", TokenType::Keyword },
        { "char", TokenType::Keyword },
        { "std::string", TokenType::Keyword },
    };

    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token(it->second, value, line, tokenColumn);
    }

    return Token(TokenType::Identifier, value, line, tokenColumn);
}

Token Lexer::stringLiteral()
{
    std::string value;
    int tokenColumn = column; // Capture the column where the string starts

    value += currentChar(); // Add the opening quote
    advance();              // Skip the opening quote

    while (currentChar() != '"' && currentChar() != '\0') {
        if (currentChar() == '\\' && peekChar() == '"') {
            value += currentChar();
            advance();
            value += currentChar(); // Add the escaped quote
        } else {
            value += currentChar();
        }
        advance();
    }

    value += currentChar(); // Add the closing quote
    advance();              // Skip the closing quote

    return Token(TokenType::StringLiteral, value, line, tokenColumn);
}

Token Lexer::characterLiteral()
{
    std::string value;
    int tokenColumn = column; // Capture the column where the char starts

    value += currentChar(); // Add the opening single quote
    advance();

    if (currentChar() == '\\' && peekChar() == '\'') {
        value += currentChar(); // Handle escape sequences like '\''
        advance();
    } else {
        value += currentChar(); // Add the actual character
    }
    advance();

    if (currentChar() != '\'') {
        throw std::runtime_error(
          "Expected closing single quote for character literal");
    }

    value += currentChar(); // Add the closing single quote
    advance();

    return Token(TokenType::CharacterLiteral, value, line, tokenColumn);
}

constexpr bool Lexer::isOperator(char c) noexcept
{
    constexpr std::string_view operators = "+-*/%=<>!&|^~";
    return operators.find(c) != std::string_view::npos;
}

Token Lexer::operatorToken()
{
    std::string value;
    int tokenColumn = column;

    while (isOperator(currentChar())) {
        value += currentChar();
        advance();

        static const std::unordered_map<std::string_view, char>
          multiCharOperators{
              { "=", '=' }, { "!", '=' }, { "<", '=' }, { ">", '=' }
          };

        auto it = multiCharOperators.find(value);
        if (it != multiCharOperators.end() && currentChar() == it->second) {
            value += currentChar();
            advance();
            break;
        }
    }

    return Token(TokenType::Operator, value, line, tokenColumn);
}

Token Lexer::nextToken()
{
    skipWhitespace();
    skipComment();

    if (std::isdigit(currentChar())) {
        return number();
    }

    if (std::isalpha(currentChar()) || currentChar() == '_') {
        return identifierOrKeyword();
    }

    if (currentChar() == '"') {
        return stringLiteral();
    }

    if (currentChar() == '\'') {
        return characterLiteral();
    }

    if (isOperator(currentChar())) {
        return operatorToken();
    }

    if (currentChar() == ';' || currentChar() == ',' || currentChar() == '(' ||
        currentChar() == ')' || currentChar() == '{' || currentChar() == '}') {
        std::string value(1, currentChar());
        Token token(TokenType::Separator, value, line, column);
        advance();
        return token;
    }

    if (currentChar() == '\0') {
        return Token(TokenType::EndOfFile, "", line, column);
    }

    // If we reach here, we have an unknown character
    char unknownChar = currentChar();
    advance();
    return Token(TokenType::Unknown, std::string(1, unknownChar), line, column);
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (index < source.size()) {
        Token token = nextToken();
        if (token.getType() != TokenType::Unknown) {
            tokens.emplace_back(token);
        } else {
            // Handle errors or unknown tokens here if needed
        }

        // Stop processing if we've reached EndOfFile to avoid adding multiple
        // EOF tokens
        if (token.getType() == TokenType::EndOfFile) {
            break;
        }
    }

    return tokens;
}
