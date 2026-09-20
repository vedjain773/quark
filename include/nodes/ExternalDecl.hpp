#ifndef EXTERNALDECL_H
#define EXTERNALDECL_H

#include "visitors/CodegenVis.hpp"
#include "visitors/Visitor.hpp"

class Expression;

class ExternalDecl {
  public:
    virtual void accept(Visitor &visitor) = 0;
    virtual void codegen(CodegenVis &codegenvis) = 0;
    virtual ~ExternalDecl() = default;
};

class GlobalDecl: public ExternalDecl {
  public:
    int line, column;
    TypeKind *type;
    std::string name;
    std::unique_ptr<Expression> expression;

    GlobalDecl(TypeKind *type, const std::string &name, std::unique_ptr<Expression> expr,
               int line, int column);

    void accept(Visitor &visitor);
    void codegen(CodegenVis &codegenvis);
};

#endif
