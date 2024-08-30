// SymbolTable.h
#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <unordered_map>
#include <string>
#include <stdexcept>

class SymbolTable
{
public:
    void declareVariable(const std::string& name, const std::string& type)
    {
        if (symbols.find(name) != symbols.end()) {
            throw std::runtime_error("Variable '" + name +
                                     "' is already declared");
        }
        symbols[name] = type;
    }

    std::string lookupVariable(const std::string& name) const
    {
        auto it = symbols.find(name);
        if (it == symbols.end()) {
            throw std::runtime_error("Variable '" + name + "' is not declared");
        }
        return it->second;
    }

private:
    std::unordered_map<std::string, std::string> symbols;
};

#endif // SYMBOL_TABLE_H
