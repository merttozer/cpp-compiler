#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <numeric>
#include "SymbolTable.hpp"

class ASTNode
{
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
    virtual void checkSemantics(SymbolTable& symTable) const = 0;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

enum class BinaryOp
{
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    LessThan,
    GreaterThan,
    Equal,
    NotEqual,
    And,
    Or
};

class Expression : public ASTNode
{
public:
    virtual std::string getType(const SymbolTable& symTable) const = 0;
    virtual void checkSemantics(SymbolTable& symTable) const override = 0;
};

using ExpressionPtr = std::shared_ptr<Expression>;

class BinaryExpression : public Expression
{
public:
    BinaryExpression(ExpressionPtr left,
                     BinaryOp op,
                     ExpressionPtr right) noexcept
      : left(std::move(left))
      , op(op)
      , right(std::move(right))
    {
    }

    std::string getType(const SymbolTable& symTable) const override
    {
        std::string leftType = left->getType(symTable);
        std::string rightType = right->getType(symTable);

        if (op == BinaryOp::And || op == BinaryOp::Or) {
            return "bool";
        }

        if ((leftType == "int" && rightType == "float") ||
            (leftType == "float" && rightType == "int")) {
            return "float";
        }

        if (leftType != rightType) {
            throw std::runtime_error(
              "Type mismatch in binary expression: " + leftType + " " +
              opToString(op) + " " + rightType);
        }

        return leftType;
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        left->checkSemantics(symTable);
        right->checkSemantics(symTable);
    }

    std::string toString() const override
    {
        return "(" + left->toString() + " " + opToString(op) + " " +
               right->toString() + ")";
    }

    ExpressionPtr getLeft() const { return left; }
    ExpressionPtr getRight() const { return right; }
    std::string getOp() const { return opToString(op); }

private:
    ExpressionPtr left;
    BinaryOp op;
    ExpressionPtr right;

    static constexpr const char* opToString(BinaryOp op) noexcept
    {
        switch (op) {
            case BinaryOp::Add:
                return "+";
            case BinaryOp::Subtract:
                return "-";
            case BinaryOp::Multiply:
                return "*";
            case BinaryOp::Divide:
                return "/";
            case BinaryOp::Modulo:
                return "%";
            case BinaryOp::LessThan:
                return "<";
            case BinaryOp::GreaterThan:
                return ">";
            case BinaryOp::Equal:
                return "==";
            case BinaryOp::NotEqual:
                return "!=";
            case BinaryOp::And:
                return "&&";
            case BinaryOp::Or:
                return "||";
            default:
                return "?";
        }
    }
};

class LiteralExpression : public Expression
{
public:
    explicit LiteralExpression(const std::string& value) noexcept
      : value(value)
    {
    }

    std::string getType(const SymbolTable&) const override
    {
        if (isCharacterLiteral())
            return "char";
        if (isStringLiteral())
            return "std::string";
        if (isFloatingPointLiteral())
            return "float";
        return "int";
    }

    void checkSemantics(SymbolTable&) const override {}

    std::string toString() const override { return value; }

    const std::string& getValue() const noexcept { return value; }

private:
    std::string value;

    bool isCharacterLiteral() const noexcept
    {
        return value.size() == 3 && value.front() == '\'' &&
               value.back() == '\'';
    }

    bool isStringLiteral() const noexcept
    {
        return value.front() == '"' && value.back() == '"';
    }

    bool isFloatingPointLiteral() const noexcept
    {
        return value.find('.') != std::string::npos;
    }
};

class VariableExpression : public Expression
{
public:
    explicit VariableExpression(const std::string& name) noexcept
      : name(name)
    {
    }

    std::string getType(const SymbolTable& symTable) const override
    {
        return symTable.lookupVariable(name);
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        symTable.lookupVariable(name);
    }

    std::string toString() const override { return name; }

    const std::string& getName() const noexcept { return name; }

private:
    std::string name;
};

class Statement : public ASTNode
{
public:
    virtual ~Statement() = default;
    virtual void checkSemantics(SymbolTable& symTable) const override = 0;
};

using StatementPtr = std::shared_ptr<Statement>;

class BlockStatement : public Statement
{
public:
    explicit BlockStatement(const std::vector<StatementPtr>& statements)
      : statements(statements)
    {
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        for (const auto& stmt : statements) {
            stmt->checkSemantics(symTable);
        }
    }

    std::string toString() const override
    {
        return std::accumulate(
          statements.begin(),
          statements.end(),
          std::string(),
          [](const std::string& acc, const StatementPtr& stmt) {
              return acc + "  " + stmt->toString() + "\n";
          });
    }

    const std::vector<StatementPtr>& getStatements() const noexcept
    {
        return statements;
    }

private:
    std::vector<StatementPtr> statements;
};

class VariableDeclaration : public Statement
{
public:
    VariableDeclaration(std::string type,
                        std::string name,
                        ExpressionPtr initializer = nullptr)
      : type(std::move(type))
      , name(std::move(name))
      , initializer(std::move(initializer))
    {
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        symTable.declareVariable(name, type);

        if (initializer) {
            initializer->checkSemantics(symTable);
            std::string initType = initializer->getType(symTable);

            // Allow type promotion in assignments
            if (initType == "int" && type == "float") {
                // Promote int to float
                initType = "float";
            } else if (initType == "float" && type == "int") {
                throw std::runtime_error(
                  "Cannot assign float to int without explicit cast");
            }

            if (initType != type) {
                throw std::runtime_error(
                  "Type mismatch: Cannot initialize variable of type '" + type +
                  "' with value of type '" + initType + "'");
            }
        }
    }

    std::string toString() const override
    {
        return type + " " + name + " = " +
               (initializer ? initializer->toString() : "null") + ";";
    }

    const std::string& getName() const noexcept { return name; }

    const ExpressionPtr& getInitializer() const noexcept { return initializer; }

private:
    std::string type;
    std::string name;
    ExpressionPtr initializer;
};

class AssignmentStatement : public Statement
{
public:
    AssignmentStatement(std::string name, ExpressionPtr value)
      : name(std::move(name))
      , value(std::move(value))
    {
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        value->checkSemantics(symTable);
        std::string varType = symTable.lookupVariable(name);
        std::string valueType = value->getType(symTable);

        // Allow type promotion in assignments
        if (valueType == "int" && varType == "float") {
            // Promote int to float
            valueType = "float";
        } else if (valueType == "float" && varType == "int") {
            throw std::runtime_error(
              "Cannot assign float to int without explicit cast");
        }

        if (varType != valueType) {
            throw std::runtime_error(
              "Type mismatch in assignment: Cannot assign " + valueType +
              " to " + varType);
        }
    }

    std::string toString() const override
    {
        return name + " = " + value->toString() + ";";
    }

    const std::string& getName() const noexcept { return name; }

    const ExpressionPtr& getValue() const noexcept { return value; }

private:
    std::string name;
    ExpressionPtr value;
};

class ReturnStatement : public Statement
{
public:
    explicit ReturnStatement(ExpressionPtr value = nullptr)
      : value(std::move(value))
    {
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        if (value) {
            value->checkSemantics(symTable);
        }
    }

    std::string toString() const override
    {
        return "return " + value->toString() + ";";
    }

    const ExpressionPtr& getReturnValue() const noexcept { return value; }

private:
    ExpressionPtr value;
};

class FunctionDeclaration : public Statement
{
public:
    FunctionDeclaration(const std::string& returnType,
                        const std::string& name,
                        const std::vector<std::string>& parameters,
                        const std::vector<StatementPtr>& body) noexcept
      : returnType(returnType)
      , name(name)
      , parameters(parameters)
      , body(body)
    {
    }

    // checkSemantics marked noexcept because it doesn't throw exceptions
    void checkSemantics(SymbolTable& symTable) const noexcept override
    {
        for (const auto& stmt : body) {
            stmt->checkSemantics(symTable);
        }
    }

    // toString method rewritten using std::stringstream for efficiency
    std::string toString() const noexcept override
    {
        std::stringstream result;
        result << returnType << " " << name << "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0)
                result << ", ";
            result << parameters[i];
        }
        result << ")";
        return result.str();
    }

    std::string getName() const { return name; }

    const std::vector<StatementPtr>& getBody() const { return body; }

private:
    std::string returnType;
    std::string name;
    std::vector<std::string> parameters;
    std::vector<StatementPtr> body;
};

class IfStatement : public Statement
{
public:
    IfStatement(ExpressionPtr condition,
                StatementPtr thenBranch,
                StatementPtr elseBranch = nullptr) noexcept
      : condition(std::move(condition))
      , thenBranch(std::move(thenBranch))
      , elseBranch(std::move(elseBranch))
    {
    }

    void checkSemantics(SymbolTable& symTable) const override
    {
        // Check the semantics of the condition
        condition->checkSemantics(symTable);

        // Ensure the condition is a boolean expression
        auto conditionType = condition->getType(symTable);
        if (conditionType != "int" && conditionType != "bool") {
            throw std::runtime_error(
              "Condition in 'if' statement must be of type int or bool");
        }

        // Check the semantics of the then branch
        thenBranch->checkSemantics(symTable);

        // Check the semantics of the else branch (if it exists)
        if (elseBranch) {
            elseBranch->checkSemantics(symTable);
        }
    }

    std::string toString() const override
    {
        std::string result =
          "if (" + condition->toString() + ") " + thenBranch->toString();
        if (elseBranch) {
            result += " else " + elseBranch->toString();
        }
        return result;
    }

    const ExpressionPtr& getCondition() const noexcept { return condition; }

    const StatementPtr& getThenBranch() const noexcept { return thenBranch; }

    const StatementPtr& getElseBranch() const noexcept { return elseBranch; }

private:
    ExpressionPtr condition;
    StatementPtr thenBranch;
    StatementPtr elseBranch;
};

#endif // AST_HPP
