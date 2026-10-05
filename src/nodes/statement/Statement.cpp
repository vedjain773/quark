#include "nodes/Statement.hpp"
#include "visitors/Visitor.hpp"

Statement::Statement(int tline, int tcol) :
    line(tline),
    column(tcol) {}

void EmptyStmt::accept(Visitor &visitor) {
    visitor.visitEmptyStmt(*this);
}

void EmptyStmt::codegen(CodegenVis &codegenvis) {
    // do nothing
}

ExprStmt::ExprStmt(std::unique_ptr<Expression> expr) :
    Statement(expr->line, expr->column),
    expression(std::move(expr)) {}

void ExprStmt::accept(Visitor &visitor) {
    visitor.visitExprStmt(*this);
}

void BlockStmt::addStmt(std::unique_ptr<Statement> stmt) {
    statements.push_back(std::move(stmt));

    Statement *statmt = (statements[0]).get();

    if (statmt == nullptr) return;

    line = statmt->line;
    column = statmt->column;
}

void BlockStmt::accept(Visitor &visitor) {
    visitor.visitBlockStmt(*this);
}

IfStmt::IfStmt(std::unique_ptr<Expression> condn, std::unique_ptr<Statement> ifbody,
               std::unique_ptr<Statement> elsestmt) :
    Statement(condn->line, condn->column),
    condition(std::move(condn)),
    body(std::move(ifbody)),
    elseStmt(std::move(elsestmt)) {}

void IfStmt::accept(Visitor &visitor) {
    visitor.visitIfStmt(*this);
}

ElseStmt::ElseStmt(std::unique_ptr<Statement> elsebody) :
    Statement(elsebody->line, elsebody->column),
    body(std::move(elsebody)) {}

void ElseStmt::accept(Visitor &visitor) {
    visitor.visitElseStmt(*this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expression> condn, std::unique_ptr<Statement> whilebody) :
    Statement(condn->line, condn->column),
    condition(std::move(condn)),
    body(std::move(whilebody)) {}

void WhileStmt::accept(Visitor &visitor) {
    visitor.visitWhileStmt(*this);
}

ForStmt::ForStmt(std::unique_ptr<Statement> init, std::unique_ptr<Expression> condn,
                 std::unique_ptr<Expression> iter, std::unique_ptr<Statement> body) :
    Statement(init->line, init->column),
    condn(std::move(condn)),
    iter(std::move(iter)),
    init(std::move(init)),
    body(std::move(body)) {}

void ForStmt::accept(Visitor &visitor) {
    visitor.visitForStmt(*this);
}

BreakStmt::BreakStmt(int tline, int tcol) :
    Statement(tline, tcol) {}

void BreakStmt::accept(Visitor &visitor) {
    visitor.visitBreakStmt(*this);
}

bool BreakStmt::isTerminator() {
    return true;
}

ContinueStmt::ContinueStmt(int tline, int tcol) :
    Statement(tline, tcol) {}

void ContinueStmt::accept(Visitor &visitor) {
    visitor.visitContinueStmt(*this);
}

bool ContinueStmt::isTerminator() {
    return true;
}

ReturnStmt::ReturnStmt(std::unique_ptr<Expression> retexpr) :
    Statement(retexpr->line, retexpr->column),
    retExpr(std::move(retexpr)) {}

void ReturnStmt::accept(Visitor &visitor) {
    visitor.visitReturnStmt(*this);
}

bool ReturnStmt::isTerminator() {
    return true;
}

DeclStmt::DeclStmt(TypeKind *tk, const std::string &varname, std::unique_ptr<Expression> expr,
                   int tline, int tcol) :
    Statement(tline, tcol),
    type(tk),
    name(varname),
    expression(std::move(expr)) {}

void DeclStmt::accept(Visitor &visitor) {
    visitor.visitDeclStmt(*this);
}

StructField::StructField(TypeKind *tk, const std::string &fieldName, int tline, int tcol) {
    type = tk;
    fName = fieldName;
    line = tline;
    column = tcol;
}

void StructField::accept(Visitor &visitor) {
    visitor.visitStructField(*this);
}

StructDecl::StructDecl(const std::string &tagName, int tline, int tcol) {
    tag = tagName;
    line = tline;
    column = tcol;
}

void StructDecl::addField(std::unique_ptr<StructField> field) {
    fields.push_back(std::move(field));
}

void StructDecl::accept(Visitor &visitor) {
    visitor.visitStructDecl(*this);
}
