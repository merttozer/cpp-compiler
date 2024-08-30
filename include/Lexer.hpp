// Lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.hpp"

class Lexer
{
public:
    Lexer();

    void setSource(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source;
    size_t index = 0;
    int line = 1;
    int column = 1;

    char currentChar() const noexcept;
    char peekChar(int offset = 1) const noexcept;
    void advance() noexcept;
    void skipWhitespace() noexcept;
    void skipComment();
    Token number();
    Token identifierOrKeyword();
    Token stringLiteral();
    Token characterLiteral();
    Token nextToken();
    static constexpr bool isOperator(char c) noexcept;
    Token operatorToken();
};

#endif // LEXER_H
