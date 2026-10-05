#include "nodes/Function.hpp"
#include <cstddef>
#include <iostream>

using size_t = std::size_t;

Parameter::Parameter(TypeKind *p_type, const std::string &p_name) :
    type(p_type),
    name(p_name) {}

void Parameter::accept(Visitor &visitor) {
    visitor.visitParameter(*this);
}

Prototype::Prototype(TypeKind *ret_type, const std::string &func_name, int tline, int tcol) :
    retType(ret_type),
    funcName(func_name),
    line(tline),
    column(tcol) {}

void Prototype::addParam(std::unique_ptr<Parameter> param) {
    paramList.push_back(std::move(param));
}

void Prototype::accept(Visitor &visitor) {
    visitor.visitPrototype(*this);
}

FuncDef::FuncDef(std::unique_ptr<Prototype> proto_type, std::unique_ptr<BlockStmt> func_body) :
    prototype(std::move(proto_type)),
    funcBody(std::move(func_body)) {}

void FuncDef::accept(Visitor &visitor) {
    visitor.visitFuncDef(*this);
}
