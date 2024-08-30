// Compiler.hpp
#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "Lexer.hpp"
#include "Parser.hpp"
#include "IRGenerator.hpp"

class Compiler
{
public:
    Compiler(std::shared_ptr<Lexer> lexer,
             std::shared_ptr<Parser> parser,
             std::shared_ptr<IRGenerator> irGenerator);

    void compile(const std::string& inputFilePath,
                 const std::string& outputFilePath);

private:
    std::shared_ptr<Lexer> lexer_;
    std::shared_ptr<Parser> parser_;
    std::shared_ptr<IRGenerator> irGenerator_;

    static std::string readFile(const std::string& filePath);
    static void writeAssemblyToFile(const std::vector<TACInstruction>& ir,
                                    const std::string& filePath);
};

#endif // COMPILER_HPP