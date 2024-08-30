#include "Compiler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Compiler::Compiler(std::shared_ptr<Lexer> lexer,
                   std::shared_ptr<Parser> parser,
                   std::shared_ptr<IRGenerator> irGenerator)
  : lexer_(std::move(lexer))
  , parser_(std::move(parser))
  , irGenerator_(std::move(irGenerator))
{
}

std::string Compiler::readFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Compiler::writeAssemblyToFile(const std::vector<TACInstruction>& ir,
                                   const std::string& filePath)
{
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        throw std::runtime_error("Could not open output file: " + filePath);
    }

    for (const auto& instruction : ir) {
        outFile << instruction.op << " " << instruction.arg1 << " "
                << instruction.arg2 << " " << instruction.result << "\n";
    }
}

void Compiler::compile(const std::string& inputFilePath,
                       const std::string& outputFilePath)
{
    std::string sourceCode = readFile(inputFilePath);

    lexer_->setSource(sourceCode);
    auto tokens = lexer_->tokenize();
    parser_->setTokens(std::move(tokens));
    auto ast = parser_->parse();
    auto ir = irGenerator_->generateCode(ast);

    writeAssemblyToFile(ir, outputFilePath);
}