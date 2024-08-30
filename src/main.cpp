#include <iostream>
#include <memory>
#include "Compiler.hpp"

int main(int argc, const char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.cpp> <output.asm>\n";
        return 1;
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = argv[2];

    try {
        auto lexer = std::make_shared<Lexer>();
        auto parser = std::make_shared<Parser>(lexer);
        auto irGenerator = std::make_shared<IRGenerator>(parser);

        Compiler compiler(lexer, parser, irGenerator);
        compiler.compile(inputFilePath, outputFilePath);
        std::cout << "Compilation successful. Assembly written to "
                  << outputFilePath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Compilation failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
