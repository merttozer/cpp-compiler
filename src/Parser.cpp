// Parser.cpp
#include "Parser.hpp"

Parser::Parser(std::shared_ptr<Lexer> lexer)
  : lexer(std::move(lexer))
  , index(0) // Initialize index here
{
}

void Parser::setTokens(std::vector<Token> tokens)
{
    this->tokens = std::move(tokens);
    this->index = 0;
}

Token Parser::currentToken() const noexcept
{
    if (index < tokens.size()) {
        return tokens[index];
    }
    return Token(TokenType::EndOfFile, "", 0, 0);
}

void Parser::advance() noexcept
{
    if (index < tokens.size()) {
        index++;
    }
}

bool Parser::match(TokenType type) const noexcept
{
    return currentToken().getType() == type;
}

StatementPtr Parser::parse()
{
    StatementPtr ast = parseStatement();

    // After parsing, perform semantic analysis
    SymbolTable symTable;
    ast->checkSemantics(symTable);

    return ast;
}

// Update parseStatement() to handle block statements
StatementPtr Parser::parseStatement()
{
    if (match(TokenType::Separator) && currentToken().getValue() == "{") {
        return parseBlockStatement();
    }

    if (match(TokenType::Keyword)) {
        const auto& value = currentToken().getValue();
        if (value == "int" || value == "float" || value == "char" ||
            value == "std::string") {
            return parseVariableDeclaration();
        } else if (value == "return") {
            return parseReturn();
        } else if (value == "if") {
            return parseIfStatement();
        }
    } else if (match(TokenType::Identifier)) {
        return parseAssignmentOrFunctionCall();
    }

    throw std::runtime_error("Unexpected token: " + currentToken().toString());
}

// New method to parse block statements
StatementPtr Parser::parseBlockStatement()
{
    advance(); // Skip '{'

    std::vector<StatementPtr> statements;
    statements.reserve(10); // Reserve space to avoid multiple reallocations

    while (!match(TokenType::Separator) || currentToken().getValue() != "}") {
        statements.push_back(parseStatement());
    }

    advance(); // Skip '}'

    return std::make_shared<BlockStatement>(std::move(statements));
}

// Updated parseIfStatement() to handle block statements in 'if' branches
StatementPtr Parser::parseIfStatement()
{
    advance(); // Skip 'if'

    if (!match(TokenType::Separator) || currentToken().getValue() != "(") {
        throw std::runtime_error("Expected '(' after 'if'");
    }
    advance(); // Skip '('

    ExpressionPtr condition = parseExpression();

    if (!match(TokenType::Separator) || currentToken().getValue() != ")") {
        throw std::runtime_error("Expected ')' after 'if' condition");
    }
    advance(); // Skip ')'

    StatementPtr thenBranch = parseStatement();

    StatementPtr elseBranch = nullptr;
    if (match(TokenType::Keyword) && currentToken().getValue() == "else") {
        advance(); // Skip 'else'
        elseBranch = parseStatement();
    }

    return std::make_shared<IfStatement>(
      std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

StatementPtr Parser::parseAssignmentOrFunctionCall()
{
    std::string name = currentToken().getValue();
    advance();

    if (match(TokenType::Operator) && currentToken().getValue() == "=") {
        advance();
        ExpressionPtr value = parseExpression();

        if (!match(TokenType::Separator) || currentToken().getValue() != ";") {
            throw std::runtime_error("Expected ';' after assignment");
        }

        advance(); // Skip ';'
        return std::make_shared<AssignmentStatement>(std::move(name),
                                                     std::move(value));
    }

    if (match(TokenType::Separator) && currentToken().getValue() == "(") {
        throw std::runtime_error("Function calls not yet supported.");
    }

    throw std::runtime_error("Unexpected token after identifier: " +
                             currentToken().toString());
}

StatementPtr Parser::parseVariableDeclaration()
{
    std::string type = currentToken().getValue();
    advance();

    if (!match(TokenType::Identifier)) {
        throw std::runtime_error(
          "Expected identifier after type in variable declaration");
    }

    std::string name = currentToken().getValue();
    advance();

    if (match(TokenType::Separator) && currentToken().getValue() == "(") {
        return parseFunctionDeclaration(std::move(type), std::move(name));
    }

    ExpressionPtr initializer = nullptr;
    if (match(TokenType::Operator) && currentToken().getValue() == "=") {
        advance();
        initializer = parseExpression();
    }

    if (!match(TokenType::Separator) || currentToken().getValue() != ";") {
        throw std::runtime_error("Expected ';' after variable declaration");
    }

    advance(); // Skip ';'
    return std::make_shared<VariableDeclaration>(
      std::move(type), std::move(name), std::move(initializer));
}

StatementPtr Parser::parseFunctionDeclaration(std::string returnType,
                                              std::string name)
{
    advance(); // Skip '('

    std::vector<std::string> parameters;
    parameters.reserve(5); // Reserve space to avoid multiple reallocations

    while (!match(TokenType::Separator) || currentToken().getValue() != ")") {
        if (match(TokenType::Identifier)) {
            std::string paramType = currentToken().getValue();
            advance();

            if (!match(TokenType::Identifier)) {
                throw std::runtime_error(
                  "Expected parameter name after type in function declaration");
            }

            std::string paramName = currentToken().getValue();
            advance();

            parameters.push_back(paramType + " " + paramName);

            if (match(TokenType::Separator) &&
                currentToken().getValue() == ",") {
                advance(); // Skip ','
            } else {
                break;
            }
        } else {
            throw std::runtime_error(
              "Expected parameter type in function declaration");
        }
    }

    if (!match(TokenType::Separator) || currentToken().getValue() != ")") {
        throw std::runtime_error("Expected ')' after function parameters");
    }

    advance(); // Skip ')'

    if (!match(TokenType::Separator) || currentToken().getValue() != "{") {
        throw std::runtime_error(
          "Expected '{' at the beginning of function body");
    }

    advance(); // Skip '{'

    std::vector<StatementPtr> body;
    body.reserve(10); // Reserve space to avoid multiple reallocations

    while (!match(TokenType::Separator) || currentToken().getValue() != "}") {
        body.push_back(parseStatement());
    }

    advance(); // Skip '}'

    return std::make_shared<FunctionDeclaration>(std::move(returnType),
                                                 std::move(name),
                                                 std::move(parameters),
                                                 std::move(body));
}

StatementPtr Parser::parseReturn()
{
    advance(); // Skip 'return'

    ExpressionPtr value = parseExpression();

    if (!match(TokenType::Separator) || currentToken().getValue() != ";") {
        throw std::runtime_error("Expected ';' after return statement");
    }

    advance(); // Skip ';'
    return std::make_shared<ReturnStatement>(std::move(value));
}

ExpressionPtr Parser::parseExpression()
{
    return parseBinaryExpression();
}

ExpressionPtr Parser::parsePrimaryExpression()
{
    if (match(TokenType::NumberLiteral) ||
        match(TokenType::FloatingPointLiteral) ||
        match(TokenType::StringLiteral) || match(TokenType::CharacterLiteral)) {
        std::string value = currentToken().getValue();
        advance();
        return std::make_shared<LiteralExpression>(std::move(value));
    }

    if (match(TokenType::Identifier)) {
        std::string name = currentToken().getValue();
        advance();
        return std::make_shared<VariableExpression>(std::move(name));
    }

    throw std::runtime_error("Unexpected token in expression: " +
                             currentToken().toString());
}

ExpressionPtr Parser::parseBinaryExpression(int precedence)
{
    ExpressionPtr left = parsePrimaryExpression();

    while (true) {
        int tokenPrecedence = getPrecedence(currentToken());

        if (tokenPrecedence < precedence) {
            return left;
        }

        BinaryOp op = tokenToBinaryOp(currentToken());
        advance();

        ExpressionPtr right = parseBinaryExpression(tokenPrecedence + 1);
        left = std::make_shared<BinaryExpression>(
          std::move(left), op, std::move(right));
    }
}

int Parser::getPrecedence(const Token& token) noexcept
{
    if (token.getType() == TokenType::Operator) {
        const std::string_view& value = token.getValue();
        if (value == "+" || value == "-")
            return 10;
        if (value == "*" || value == "/" || value == "%")
            return 20;
        if (value == "==" || value == "!=")
            return 5;
        if (value == "&&" || value == "||")
            return 3;
        if (value == "<" || value == ">" || value == "<=" || value == ">=")
            return 15;
    }
    return -1;
}

BinaryOp Parser::tokenToBinaryOp(const Token& token)
{
    const std::string_view& value = token.getValue();
    if (value == "+")
        return BinaryOp::Add;
    if (value == "-")
        return BinaryOp::Subtract;
    if (value == "*")
        return BinaryOp::Multiply;
    if (value == "/")
        return BinaryOp::Divide;
    if (value == "%")
        return BinaryOp::Modulo;
    if (value == "<")
        return BinaryOp::LessThan;
    if (value == ">")
        return BinaryOp::GreaterThan;
    if (value == "==")
        return BinaryOp::Equal;
    if (value == "!=")
        return BinaryOp::NotEqual;
    if (value == "&&")
        return BinaryOp::And;
    if (value == "||")
        return BinaryOp::Or;

    throw std::runtime_error("Unknown binary operator: " + token.toString());
}