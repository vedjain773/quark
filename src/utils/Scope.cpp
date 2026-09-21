#include "utils/Scope.hpp"
#include <format>
#include <iostream>

std::unordered_map<std::string, std::unique_ptr<TypeKind>> typeTable = [] {
    std::unordered_map<std::string, std::unique_ptr<TypeKind>> m;

    TypeKind intType = {
        .type = TypeEnum::BUILTIN,
        .name = "int",
        .size = 4,
        .align = 4,
        .to = nullptr
    }; 

    TypeKind uint8Type = {
        .type = TypeEnum::BUILTIN,
        .name = "uint8_t",
        .size = 1,
        .align = 1,
        .isSigned = false,
        .to = nullptr
    };

    TypeKind uint16Type = {
        .type = TypeEnum::BUILTIN,
        .name = "uint16_t",
        .size = 2,
        .align = 2,
        .isSigned = false,
        .to = nullptr
    };

    TypeKind charType = {
        .type = TypeEnum::BUILTIN,
        .name = "char",
        .size = 1,
        .align = 1,
        .to = nullptr
    };

    TypeKind voidType = {
        .type = TypeEnum::BUILTIN,
        .name = "void",
        .size = 0,
        .align = 0,
        .to = nullptr
    };

    TypeKind nullType = {
        .type = TypeEnum::BUILTIN,
        .name = "null",
        .size = 0,
        .align = 0,
        .to = nullptr
    };

    TypeKind errType = {
        .type = TypeEnum::ERROR,
        .name = "error",
        .size = 0,
        .align = 0,
        .to = nullptr
    };
        
    m.emplace("int", std::make_unique<TypeKind>(intType));
    m.emplace("uint8_t", std::make_unique<TypeKind>(uint8Type));
    m.emplace("uint16_t", std::make_unique<TypeKind>(uint16Type));
    m.emplace("char", std::make_unique<TypeKind>(charType));
    m.emplace("void", std::make_unique<TypeKind>(voidType));
    m.emplace("null", std::make_unique<TypeKind>(nullType));
    m.emplace("error", std::make_unique<TypeKind>(errType));

    return m;
}();

TypeKind *getType(const std::string &typeName) {
    int size = typeName.size();

    if (typeTable.count(typeName) != 0) {
        return typeTable[typeName].get();
    } else if (typeName[size - 1] == '*') {
        TypeKind *base = typeTable[typeName.substr(0, size - 1)].get();

        TypeKind ptrType = {
            .type = TypeEnum::POINTER,
            .name = typeName,
            .size = 8,
            .align = 8,
            .to = base
        };

        std::unique_ptr<TypeKind> newType = std::make_unique<TypeKind>(ptrType);

        TypeKind *newType_raw = newType.get();

        typeTable[typeName] = std::move(newType);
        return newType_raw;
    }

    return typeTable["null"].get();
}

TypeKind *getArrType(const std::string &typeName, int numOfElements) {
    TypeKind *base = getType(typeName);
    size_t baseSize = base->size;

    size_t arrSize = numOfElements * baseSize;

    std::string newTypeName = std::format("{}[{}]", typeName, numOfElements);

    if (typeTable.count(newTypeName)) return typeTable[newTypeName].get();
        
    TypeKind arrType = {
        .type = TypeEnum::ARRAY, 
        .name = newTypeName,
        .size = arrSize,
        .align = base->align,
        .to = base 
    };

    std::unique_ptr<TypeKind> newType = std::make_unique<TypeKind>(arrType);

    TypeKind *newType_raw = newType.get();

    typeTable[newTypeName] = std::move(newType);
    return newType_raw;
}

TypeKind *createStructType(const std::string &tag) {
    std::string typeName = "struct ";
    typeName += tag;

    TypeKind structType = {
        .type = TypeEnum::STRUCT,
        .name = typeName,
        .size = 1,
        .align = 8,
        .to = nullptr
    };

    std::unique_ptr<TypeKind> newType =std::make_unique<TypeKind>(structType);

    typeTable[typeName] = std::move(newType);
    return typeTable[typeName].get();
}

bool isPointerType(TypeKind *typek) {
    return typek->type == TypeEnum::POINTER;
}

bool isArrayType(TypeKind *typek) {
    return typek->type == TypeEnum::ARRAY;
}

bool isStructType(TypeKind *typek) {
    return typek->type == TypeEnum::STRUCT;
}

bool isErrorType(TypeKind *typek) {
    return typek->type == TypeEnum::ERROR;
}

int getNumElements(TypeKind *typek) {
    int arrSize = typek->size;
    int elementSize = typek->to->size;

    return (arrSize / elementSize);
}

void Scope::addRow(const std::string &name, TypeKind *type, SymbolKind symKind) {
    Symbol symbol;
    symbol.type = type;
    symbol.kind = symKind;

    symTable.insert({name, symbol});
}

bool Scope::search(const std::string &name) {
    return symTable.count(name);
}

void Scope::addParam(const std::string &name, TypeKind *type) {
    Symbol &sym = symTable[name];
    sym.params.push_back(type);
}

size_t Scope::getNumParams(const std::string &name) {
    Symbol &sym = symTable[name];
    return sym.params.size();
}

TypeKind *Scope::getSymType(const std::string &name) {
    Symbol &sym = symTable[name];
    return sym.type;
}

SymbolKind Scope::getSymKind(const std::string &name) {
    Symbol &sym = symTable[name];
    return sym.kind;
}

std::vector<TypeKind *> Scope::getParams(const std::string &name) {
    Symbol &sym = symTable[name];
    return sym.params;
}
