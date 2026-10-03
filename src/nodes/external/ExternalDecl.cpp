#include "nodes/ExternalDecl.hpp"
#include "nodes/Expression.hpp"
#include "visitors/CodegenVis.hpp"
#include "visitors/Visitor.hpp"

GlobalDecl::GlobalDecl(TypeKind *type, const std::string &name, std::unique_ptr<Expression> expr,
        int line, int column)
    : line(line),
      column(column),
      type(type),
      name(name),
      expression(std::move(expr)) {}

void GlobalDecl::accept(Visitor &visitor) {
    visitor.visitGlobalDecl(*this);
}
