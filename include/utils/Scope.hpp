#ifndef SCOPE_H
#define SCOPE_H

#include "frontend/scanner/Token.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

using size_t = std::size_t;

enum class SymbolKind { VARIABLE, FUNCTION };

enum class TypeEnum {
    // Primitives
    BUILTIN,

    // Aggregate
    POINTER,
    ARRAY,
    STRUCT,

    // Error
    ERROR
};

struct TypeKind {
    TypeEnum type;

    std::string name;
    size_t size;
    size_t align;
    bool isSigned = true;

    TypeKind *to = nullptr;

    struct Field {
        TypeKind *fType;
        std::string name;
    };

    std::vector<Field> fields;
};

struct Symbol {
    SymbolKind kind;
    TypeKind *type;

    std::vector<TypeKind *> params;
};

extern std::unordered_map<std::string, std::unique_ptr<TypeKind>> typeTable;

TypeKind *getType(const std::string &typeName);
TypeKind *getArrType(const std::string &typeName, int numOfElements);
TypeKind *createStructType(const std::string &tag);

bool isPointerType(TypeKind *typek);
bool isArrayType(TypeKind *typek);
bool isStructType(TypeKind *typek);
bool isErrorType(TypeKind *typek);

int getNumElements(TypeKind *typek);

class Scope {
  public:
    std::map<std::string, Symbol> symTable;

    void addRow(const std::string &name, TypeKind *type, SymbolKind symKind);
    bool search(const std::string &name);

    void addParam(const std::string &name, TypeKind *type);
    size_t getNumParams(const std::string &name);
    TypeKind *getSymType(const std::string &name);
    SymbolKind getSymKind(const std::string &name);
    std::vector<TypeKind *> getParams(const std::string &name);
};

#endif
