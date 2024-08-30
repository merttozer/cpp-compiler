// Parser.h
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include "Token.hpp"
#include "AST.hpp"
#include "Lexer.hpp"

class Parser
{
public:
    explicit Parser(std::shared_ptr<Lexer> lexer);

    void setTokens(std::vector<Token> tokens);

    StatementPtr parse();

private:
    std::shared_ptr<Lexer> lexer;
    std::vector<Token> tokens;
    size_t index;

    Token currentToken() const noexcept;
    void advance() noexcept;
    bool match(TokenType type) const noexcept;

    StatementPtr parseStatement();
    StatementPtr parseVariableDeclaration();
    StatementPtr parseFunctionDeclaration(std::string returnType,
                                          std::string name);
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseDoWhileStatement();
    StatementPtr parseBlockStatement();
    StatementPtr parseAssignmentOrFunctionCall();
    StatementPtr parseReturn();

    ExpressionPtr parseExpression();
    ExpressionPtr parsePrimaryExpression();
    ExpressionPtr parseBinaryExpression(int precedence = 0);

    static int getPrecedence(const Token& token) noexcept;
    static BinaryOp tokenToBinaryOp(const Token& token);
};

#endif // PARSER_HPP
