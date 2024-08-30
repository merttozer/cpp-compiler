// IntermediateCode.cpp
#include "IRGenerator.hpp"

IRGenerator::IRGenerator(std::shared_ptr<Parser> parser)
  : parser(std::move(parser))
{
}

std::vector<TACInstruction> IRGenerator::generateCode(const ASTNodePtr& ast)
{
    code.reserve(100); // Reserve space to reduce reallocations

    if (auto block = std::dynamic_pointer_cast<BlockStatement>(ast)) {
        for (const auto& stmt : block->getStatements()) {
            generateStatement(stmt);
        }
    } else {
        generateStatement(std::dynamic_pointer_cast<Statement>(ast));
    }
    return code;
}

void IRGenerator::generateStatement(const StatementPtr& stmt)
{
    if (auto varDecl = std::dynamic_pointer_cast<VariableDeclaration>(stmt)) {
        std::string resultVar = varDecl->getName();
        if (const auto& initializer = varDecl->getInitializer()) {
            std::string tempVar;
            generateExpression(initializer, tempVar);
            code.emplace_back(
              "MOV", std::move(tempVar), "", std::move(resultVar));
        }
    } else if (auto assignStmt =
                 std::dynamic_pointer_cast<AssignmentStatement>(stmt)) {
        std::string tempVar;
        generateExpression(assignStmt->getValue(), tempVar);
        code.emplace_back("MOV", std::move(tempVar), "", assignStmt->getName());
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        std::string conditionVar;
        generateExpression(ifStmt->getCondition(), conditionVar);
        code.emplace_back("IF_FALSE", std::move(conditionVar), "", "L1");

        generateStatement(ifStmt->getThenBranch());
        code.emplace_back("GOTO", "", "", "L2");

        code.emplace_back("LABEL", "", "", "L1");
        if (ifStmt->getElseBranch()) {
            generateStatement(ifStmt->getElseBranch());
        }
        code.emplace_back("LABEL", "", "", "L2");
    } else if (auto blockStmt =
                 std::dynamic_pointer_cast<BlockStatement>(stmt)) {
        for (const auto& innerStmt : blockStmt->getStatements()) {
            generateStatement(innerStmt);
        }
    } else if (auto returnStmt =
                 std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
        if (const auto& returnValue = returnStmt->getReturnValue()) {
            std::string tempVar;
            generateExpression(returnValue, tempVar);
            code.emplace_back("RET", std::move(tempVar), "", "");
        } else {
            code.emplace_back("RET", "", "", "");
        }
        return; // Stop processing further statements after a return
    } else if (auto funcDecl =
                 std::dynamic_pointer_cast<FunctionDeclaration>(stmt)) {
        code.emplace_back("LABEL", "", "", funcDecl->getName());

        for (const auto& bodyStmt : funcDecl->getBody()) {
            generateStatement(bodyStmt);
            if (!code.empty() && code.back().op == "RET") {
                return;
            }
        }

        if (code.empty() || code.back().op != "RET") {
            code.emplace_back("RET", "", "", "");
        }
    }
}

void IRGenerator::generateExpression(const ExpressionPtr& expr,
                                     std::string& resultVar)
{
    if (auto binExpr = std::dynamic_pointer_cast<BinaryExpression>(expr)) {
        std::string leftVar, rightVar;
        generateExpression(binExpr->getLeft(), leftVar);
        generateExpression(binExpr->getRight(), rightVar);
        resultVar = getNewTempVar();
        code.emplace_back(
          binExpr->getOp(), std::move(leftVar), std::move(rightVar), resultVar);
    } else if (auto litExpr =
                 std::dynamic_pointer_cast<LiteralExpression>(expr)) {
        resultVar = litExpr->getValue();
    } else if (auto varExpr =
                 std::dynamic_pointer_cast<VariableExpression>(expr)) {
        resultVar = varExpr->getName();
    }
    // Handle other expression types similarly
}

std::string IRGenerator::getNewTempVar() noexcept
{
    return "t" + std::to_string(tempVarCount++);
}