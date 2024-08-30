#ifndef IR_GENERATOR_HPP
#define IR_GENERATOR_HPP

#include <string>
#include <vector>
#include "AST.hpp"
#include "Parser.hpp"

struct TACInstruction
{
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    TACInstruction(std::string op,
                   std::string arg1,
                   std::string arg2,
                   std::string result)
      : op(std::move(op))
      , arg1(std::move(arg1))
      , arg2(std::move(arg2))
      , result(std::move(result))
    {
    }
};

class IRGenerator
{
public:
    explicit IRGenerator(std::shared_ptr<Parser> parser);

    std::vector<TACInstruction> generateCode(const ASTNodePtr& ast);

private:
    std::shared_ptr<Parser> parser;
    std::vector<TACInstruction> code;
    int tempVarCount = 0;

    void generateStatement(const StatementPtr& stmt);
    void generateExpression(const ExpressionPtr& expr, std::string& resultVar);

    std::string getNewTempVar() noexcept;
};

#endif // IR_GENERATOR_HPP
