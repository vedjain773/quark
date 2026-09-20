#include "nodes/Expression.hpp"
#include "nodes/ExternalDecl.hpp"
#include "nodes/Function.hpp"
#include "nodes/Program.hpp"
#include "nodes/Statement.hpp"
#include "utils/Error.hpp"
#include "visitors/Visitor.hpp"
#include <cstddef>
#include <iostream>

using size_t = std::size_t;

void PrintVisitor::visitIntExpr(IntExpr &intexpr) {
    std::cout << getIndent() << "|-Int(" << intexpr.Val << ")\n";
}

void PrintVisitor::visitCharExpr(CharExpr &charexpr) {
    std::cout << getIndent() << "|-Char(" << charexpr.character << ")\n";
}

void PrintVisitor::visitVarExpr(VarExpr &varexpr) {
    std::cout << getIndent() << "|-Var(" << varexpr.Name << ")\n";
}

void PrintVisitor::visitCallExpr(CallExpr &callexpr) {
    std::cout << getIndent() << "|-Call(" << callexpr.callee << ")\n";

    depth += 1;
    for (size_t i = 0; i < callexpr.args.size(); i++) {
        (callexpr.args[i])->accept(*this);
    }
    depth -= 1;
}

void PrintVisitor::visitCastExpr(CastExpr &castexpr) {
    std::cout << getIndent() << "|-Cast()\n";

    depth += 1;
    (castexpr.expr)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitDerefExpr(DerefExpr &derefexpr) {
    std::cout << getIndent() << "|-Deref(*)\n";

    depth += 1;
    (derefexpr.expr)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitAddressExpr(AddressExpr &addressexpr) {
    std::cout << getIndent() << "|-Address(&)\n";

    depth += 1;
    (addressexpr.expr)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitSizeOfExpr(SizeOfExpr &sizeofexpr) {
    std::cout << getIndent() << "|-Sizeof(" << sizeofexpr.argType->name << ")\n";
}

void PrintVisitor::visitUnaryExpr(UnaryExpr &unaryexpr) {
    std::cout << getIndent() << "|-Unary(" << getOpStr(unaryexpr.Op) << ")\n";

    depth += 1;
    (unaryexpr.Operand)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitBinaryExpr(BinaryExpr &binexpr) {
    std::cout << getIndent() << "|-Oper(" << getOpStr(binexpr.Op) << ")\n";

    depth += 1;
    (binexpr.LHS)->accept(*this);
    depth -= 1;

    depth += 1;
    (binexpr.RHS)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitAssignExpr(AssignExpr &assignexpr) {
    std::cout << getIndent() << "|-Assign(=)\n";

    depth += 1;
    (assignexpr.LHS)->accept(*this);
    depth -= 1;

    depth += 1;
    (assignexpr.RHS)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitCompAssignExpr(CompAssignExpr &compassignexpr) {
    std::cout << getIndent() << "|-CompAssign(" << getOpStr(compassignexpr.Op) << ")\n";

    depth += 1;
    (compassignexpr.LHS)->accept(*this);
    depth -= 1;

    depth += 1;
    (compassignexpr.RHS)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitEmptyExpr(EmptyExpr &emptyexpr) {
    // ignore
}

void PrintVisitor::visitMemberAccessExpr(MemberAccessExpr &memexpr) {
    std::cout << getIndent() << "|-MemAccess(" << memexpr.fName << ")\n";
}

void PrintVisitor::visitExprStmt(ExprStmt &exprstmt) {
    std::cout << getIndent() << "|-Stmt(Expr)\n";

    depth += 1;
    (exprstmt.expression)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitBlockStmt(BlockStmt &blockstmt) {
    std::cout << getIndent() << "|-Stmt(Block)\n";

    depth += 1;

    for (auto &stmt : blockstmt.statements) {
        if (stmt != nullptr)
            stmt->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitIfStmt(IfStmt &ifstmt) {
    std::cout << getIndent() << "|-Stmt(If)\n";

    depth += 1;

    (ifstmt.condition)->accept(*this);
    (ifstmt.body)->accept(*this);

    if (ifstmt.elseStmt != nullptr) {
        (ifstmt.elseStmt)->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitElseStmt(ElseStmt &elsestmt) {
    std::cout << getIndent() << "|-Stmt(Else)\n";

    depth += 1;
    (elsestmt.body)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitWhileStmt(WhileStmt &whilestmt) {
    std::cout << getIndent() << "|-Stmt(While)\n";

    depth += 1;
    (whilestmt.condition)->accept(*this);
    (whilestmt.body)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitForStmt(ForStmt &forstmt) {
    std::cout << getIndent() << "|-Stmt(For)\n";

    depth += 1;
    (forstmt.init)->accept(*this);
    (forstmt.condn)->accept(*this);
    (forstmt.iter)->accept(*this);

    (forstmt.body)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitBreakStmt(BreakStmt &breakstmt) {
    std::cout << getIndent() << "|-Stmt(Break)\n";
}

void PrintVisitor::visitContinueStmt(ContinueStmt &continuestmt) {
    std::cout << getIndent() << "|-Stmt(Continue)\n";
}

void PrintVisitor::visitReturnStmt(ReturnStmt &returnstmt) {
    std::cout << getIndent() << "|-Stmt(Return)\n";

    depth += 1;
    (returnstmt.retExpr)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitDeclStmt(DeclStmt &declstmt) {
    std::cout << getIndent() << "|-Stmt(Declare)\n";
    std::cout << getIndent() << "  |-Var(" + declstmt.name + ")\n";

    depth += 1;

    if (declstmt.expression != nullptr) {
        (declstmt.expression)->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitStructDecl(StructDecl &structdecl) {
    std::cout << getIndent() << "|-Struct(" << structdecl.tag << ")\n";

    depth += 1;

    for (auto &field : structdecl.fields) {
        field->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitStructField(StructField &structfield) {
    std::cout << getIndent() << "|-Field(" << structfield.fName << ")\n";
}

void PrintVisitor::visitEmptyStmt(EmptyStmt &emptystmt) {
    std::cout << getIndent() << "\n";
}

void PrintVisitor::visitParameter(Parameter &parameter) {
    std::cout << getIndent() << "|-Param(" << parameter.name << ")\n";
}

void PrintVisitor::visitPrototype(Prototype &prototype) {
    std::cout << getIndent() << "|-Prototype(" << prototype.funcName << ")\n";

    depth += 1;

    for (auto &param : prototype.paramList) {
        param->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitFuncDef(FuncDef &funcdef) {
    std::cout << getIndent() << "|-FuncDef\n";

    depth += 1;
    (funcdef.prototype)->accept(*this);
    (funcdef.funcBody)->accept(*this);
    depth -= 1;
}

void PrintVisitor::visitGlobalDecl(GlobalDecl &globaldecl) {
    std::cout << getIndent() << "|-Global(Declare)\n";
    std::cout << getIndent() << "  |-Var(" + globaldecl.name + ")\n";

    depth += 1;

    if (globaldecl.expression != nullptr) {
        (globaldecl.expression)->accept(*this);
    }

    depth -= 1;
}

void PrintVisitor::visitProgram(Program &program) {
    for (auto &edecl : program.root) {
        edecl->accept(*this);
    }
}

std::string PrintVisitor::getIndent() {
    return std::string(depth * 2, ' ');
}
